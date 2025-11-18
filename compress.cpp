/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			MICHAEL GALLE
Implemented:    KIEN MACARTNEY
Date:           NOV 11 2025
Comments:		Projects III - Coded Messaging System

				Huffman compression implementation file

==========================================================================================================================
*/
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Tx.h"
#include "Rx.h"
#include "ui.h"
#include "RS232Comm.h"
#include "sound.h"
#include "audioQueue.h"
#include "config.h"
#include "compress.h"
#include "encrypt.h"

/*************************************************************************
* Types used for Huffman coding
*************************************************************************/

typedef struct {                //represents a bitstream, allowing bit-by-bit reading/writing
    unsigned char* BytePtr;     //pointer to current byte in stream
    unsigned int  BitPos;       //bit position within that byte (0-7)
} huff_bitstream_t;

typedef struct {                //stores info about a single symbol
    int Symbol;                 //byte value (0-255)
    unsigned int Count;         //how many times it appears
    unsigned int Code;          //binary sequence
    unsigned int Bits;          //number of bits (length of the sequence)
} huff_sym_t;

typedef struct huff_encodenode_struct huff_encodenode_t;

struct huff_encodenode_struct {             //used when building the huffman tree
    huff_encodenode_t* ChildA, * ChildB;
    int Count;
    int Symbol;                             //if symbol >= 0, leaf node       
};                                          //if symbol == -1, internal node combining two smaller nodes

typedef struct huff_decodenode_struct huff_decodenode_t;

struct huff_decodenode_struct {             //used when decoding the huffman tree
    huff_decodenode_t* ChildA, * ChildB;    //no frequency counts
    int Symbol;
};


/*************************************************************************
* Constants for Huffman decoding
*************************************************************************/

/* The maximum number of nodes in the Huffman tree is 2^(8+1)-1 = 511 */
#define MAX_TREE_NODES 511

/*************************************************************************
*                           INTERNAL FUNCTIONS                           *
*************************************************************************/


/*************************************************************************
* _Huffman_InitBitstream() - Initialize a bitstream.
*************************************************************************/

static void _Huffman_InitBitstream(huff_bitstream_t* stream, unsigned char* buf) {
    stream->BytePtr = buf;
    stream->BitPos = 0;
} //initializes a bit stream to start reading or writing at the beginning of a buf

/*************************************************************************
* _Huffman_ReadBit() - Read one bit from a bitstream.
*************************************************************************/

static unsigned int _Huffman_ReadBit(huff_bitstream_t* stream) {
    unsigned int  x, bit;
    unsigned char* buf;

    /* Get current stream state */
    buf = stream->BytePtr;
    bit = stream->BitPos;

    /* Extract bit */
    x = (*buf & (1 << (7 - bit))) ? 1 : 0;
    bit = (bit + 1) & 7;
    if (!bit)
    {
        ++buf;
    }

    /* Store new stream state */
    stream->BitPos = bit;
    stream->BytePtr = buf;

    return x;
} //reads one bit from the current bytem updates the position

/*************************************************************************
* _Huffman_Read8Bits() - Read eight bits from a bitstream.
*************************************************************************/

static unsigned int _Huffman_Read8Bits(huff_bitstream_t* stream) {
    unsigned int  x, bit;
    unsigned char* buf;

    /* Get current stream state */
    buf = stream->BytePtr;
    bit = stream->BitPos;

    /* Extract byte */
    x = (*buf << bit) | (buf[1] >> (8 - bit));
    ++buf;

    /* Store new stream state */
    stream->BytePtr = buf;

    return x;
}

/*************************************************************************
* _Huffman_WriteBits() - Write bits to a bitstream.
*************************************************************************/

static void _Huffman_WriteBits(huff_bitstream_t* stream, unsigned int x, unsigned int bits) {
    unsigned int  bit, count;
    unsigned char* buf;
    unsigned int  mask;

    /* Get current stream state */
    buf = stream->BytePtr;
    bit = stream->BitPos;

    /* Append bits */
    mask = 1 << (bits - 1);
    for (count = 0; count < bits; ++count)
    {
        *buf = (*buf & (0xff ^ (1 << (7 - bit)))) +
            ((x & mask ? 1 : 0) << (7 - bit));
        x <<= 1;
        bit = (bit + 1) & 7;
        if (!bit)
        {
            ++buf;
        }
    }

    /* Store new stream state */
    stream->BytePtr = buf;
    stream->BitPos = bit;
}

/*************************************************************************
* _Huffman_Hist() - Calculate (sorted) histogram for a block of data.
*************************************************************************/

static void _Huffman_Hist(unsigned char* in, huff_sym_t* sym, unsigned int size) {
    int k;

    /* Clear/init histogram */
    for (k = 0; k < 256; ++k)
    {
        sym[k].Symbol = k;
        sym[k].Count = 0;
        sym[k].Code = 0;
        sym[k].Bits = 0;
    }

    /* Build histogram */
    for (k = size; k; --k)
    {
        sym[*in++].Count++;
    }
}

/*************************************************************************
* _Huffman_StoreTree() - Store a Huffman tree in the output stream and
* in a look-up-table (a symbol array).
*************************************************************************/

static void _Huffman_StoreTree(huff_encodenode_t* node, huff_sym_t* sym, huff_bitstream_t* stream, unsigned int code, unsigned int bits) {
    unsigned int sym_idx;

    /* Is this a leaf node? */
    if (node->Symbol >= 0)
    {
        /* Append symbol to tree description */
        _Huffman_WriteBits(stream, 1, 1);
        _Huffman_WriteBits(stream, node->Symbol, 8);

        /* Find symbol index */
        for (sym_idx = 0; sym_idx < 256; ++sym_idx)
        {
            if (sym[sym_idx].Symbol == node->Symbol) break;
        }

        /* Store code info in symbol array */
        sym[sym_idx].Code = code;
        sym[sym_idx].Bits = bits;
        return;
    }
    else
    {
        /* This was not a leaf node */
        _Huffman_WriteBits(stream, 0, 1);
    }

    /* Branch A */
    _Huffman_StoreTree(node->ChildA, sym, stream, (code << 1) + 0, bits + 1);

    /* Branch B */
    _Huffman_StoreTree(node->ChildB, sym, stream, (code << 1) + 1, bits + 1);
}

/*************************************************************************
* _Huffman_MakeTree() - Generate a Huffman tree.
*************************************************************************/

static void _Huffman_MakeTree(huff_sym_t* sym, huff_bitstream_t* stream) {
    huff_encodenode_t nodes[MAX_TREE_NODES], * node_1, * node_2, * root;
    unsigned int k, num_symbols, nodes_left, next_idx;

    /* Initialize all leaf nodes */
    num_symbols = 0;
    for (k = 0; k < 256; ++k)
    {
        if (sym[k].Count > 0)
        {
            nodes[num_symbols].Symbol = sym[k].Symbol;
            nodes[num_symbols].Count = sym[k].Count;
            nodes[num_symbols].ChildA = (huff_encodenode_t*)0;
            nodes[num_symbols].ChildB = (huff_encodenode_t*)0;
            ++num_symbols;
        }
    }

    /* Build tree by joining the lightest nodes until there is only
       one node left (the root node). */
    root = (huff_encodenode_t*)0;
    nodes_left = num_symbols;
    next_idx = num_symbols;
    while (nodes_left > 1)
    {
        /* Find the two lightest nodes */
        node_1 = (huff_encodenode_t*)0;
        node_2 = (huff_encodenode_t*)0;
        for (k = 0; k < next_idx; ++k)
        {
            if (nodes[k].Count > 0)
            {
                if (!node_1 || (nodes[k].Count <= node_1->Count))
                {
                    node_2 = node_1;
                    node_1 = &nodes[k];
                }
                else if (!node_2 || (nodes[k].Count <= node_2->Count))
                {
                    node_2 = &nodes[k];
                }
            }
        }

        /* Join the two nodes into a new parent node */
        root = &nodes[next_idx];
        root->ChildA = node_1;
        root->ChildB = node_2;
        root->Count = node_1->Count + node_2->Count;
        root->Symbol = -1;
        node_1->Count = 0;
        node_2->Count = 0;
        ++next_idx;
        --nodes_left;
    }

    /* Store the tree in the output stream, and in the sym[] array (the
        latter is used as a look-up-table for faster encoding) */
    if (root)
    {
        _Huffman_StoreTree(root, sym, stream, 0, 0);
    }
    else
    {
        /* Special case: only one symbol => no binary tree */
        root = &nodes[0];
        _Huffman_StoreTree(root, sym, stream, 0, 1);
    }
}

/*************************************************************************
* _Huffman_RecoverTree() - Recover a Huffman tree from a bitstream.
*************************************************************************/

static huff_decodenode_t* _Huffman_RecoverTree(huff_decodenode_t* nodes, huff_bitstream_t* stream, unsigned int* nodenum) {
    huff_decodenode_t* this_node;

    /* Pick a node from the node array */
    this_node = &nodes[*nodenum];
    *nodenum = *nodenum + 1;

    /* Clear the node */
    this_node->Symbol = -1;
    this_node->ChildA = (huff_decodenode_t*)0;
    this_node->ChildB = (huff_decodenode_t*)0;

    /* Is this a leaf node? */
    if (_Huffman_ReadBit(stream))
    {
        /* Get symbol from tree description and store in lead node */
        this_node->Symbol = _Huffman_Read8Bits(stream);

        return this_node;
    }

    /* Get branch A */
    this_node->ChildA = _Huffman_RecoverTree(nodes, stream, nodenum);

    /* Get branch B */
    this_node->ChildB = _Huffman_RecoverTree(nodes, stream, nodenum);

    return this_node;
}

/*************************************************************************
*                            PUBLIC FUNCTIONS                            *
*************************************************************************/

/*************************************************************************
* Huffman_Compress() - Compress a block of data using a Huffman coder.
*  in     - Input (uncompressed) buffer.
*  out    - Output (compressed) buffer. This buffer must be 384 bytes
*           larger than the input buffer.
*  insize - Number of input bytes.
* The function returns the size of the compressed data.
*************************************************************************/

int Huffman_Compress(unsigned char* in, unsigned char* out, unsigned int insize) {
    huff_sym_t       sym[256], tmp;
    huff_bitstream_t stream;
    unsigned int     k, total_bytes, swaps, symbol;

    /* Do we have anything to compress? */
    if (insize < 1) return 0;

    /* Initialize bitstream */
    _Huffman_InitBitstream(&stream, out);

    /* Calculate and sort histogram for input data */
    _Huffman_Hist(in, sym, insize);

    /* Build Huffman tree */
    _Huffman_MakeTree(sym, &stream);

    /* Sort histogram - first symbol first (bubble sort) */
    do
    {
        swaps = 0;
        for (k = 0; k < 255; ++k)
        {
            if (sym[k].Symbol > sym[k + 1].Symbol)
            {
                tmp = sym[k];
                sym[k] = sym[k + 1];
                sym[k + 1] = tmp;
                swaps = 1;
            }
        }
    } while (swaps);

    /* Encode input stream */
    for (k = 0; k < insize; ++k)
    {
        symbol = in[k];
        _Huffman_WriteBits(&stream, sym[symbol].Code,
            sym[symbol].Bits);
    }

    /* Calculate size of output data */
    total_bytes = (int)(stream.BytePtr - out);
    if (stream.BitPos > 0)
    {
        ++total_bytes;
    }

    return total_bytes;
}

/*************************************************************************
* Huffman_Uncompress() - Uncompress a block of data using a Huffman
* decoder.
*  in      - Input (compressed) buffer.
*  out     - Output (uncompressed) buffer. This buffer must be large
*            enough to hold the uncompressed data.
*  insize  - Number of input bytes.
*  outsize - Number of output bytes.
*************************************************************************/

void Huffman_Uncompress(unsigned char* in, unsigned char* out, unsigned int insize, unsigned int outsize) {
    huff_decodenode_t nodes[MAX_TREE_NODES], * root, * node;
    huff_bitstream_t  stream;
    unsigned int      k, node_count;
    unsigned char* buf;

    /* Do we have anything to decompress? */
    if (insize < 1) return;

    /* Initialize bitstream */
    _Huffman_InitBitstream(&stream, in);

    /* Recover Huffman tree */
    node_count = 0;
    root = _Huffman_RecoverTree(nodes, &stream, &node_count);

    /* Decode input stream */
    buf = out;
    for (k = 0; k < outsize; ++k)
    {
        /* Traverse tree until we find a matching leaf node */
        node = root;
        while (node->Symbol < 0)
        {
            /* Get next node */
            if (_Huffman_ReadBit(&stream))
                node = node->ChildB;
            else
                node = node->ChildA;
        }

        /* We found the matching leaf node and have the symbol */
        *buf++ = (unsigned char)node->Symbol;
    }
}

/*************************************************************************
* File wrapper functions implementation
*************************************************************************/

void huffman_compress_file(const char* input_file, const char* output_file) {
    FILE* in_file = fopen(input_file, "rb");
    if (!in_file) {
        printf("Cannot open input file: %s\n", input_file);
        return;
    }

    // Get file size
    fseek(in_file, 0, SEEK_END);
    long file_size = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    if (file_size <= 0) {
        printf("Empty file or error reading file size\n");
        fclose(in_file);
        return;
    }

    // Read input data
    unsigned char* input_data = (unsigned char*)malloc((size_t)file_size);
    if (!input_data) {
        printf("Memory allocation failed\n");
        fclose(in_file);
        return;
    }

    size_t bytes_read = fread(input_data, 1, (size_t)file_size, in_file);
    fclose(in_file);

    if (bytes_read != (size_t)file_size) {
        printf("Error reading input file\n");
        free(input_data);
        return;
    }

    // Compress data - Huffman needs output buffer to be 384 bytes larger
    unsigned char* compressed_data = (unsigned char*)malloc((size_t)file_size + 384);
    if (!compressed_data) {
        printf("Memory allocation failed for compressed data\n");
        free(input_data);
        return;
    }

    int compressed_size = Huffman_Compress(input_data, compressed_data, (unsigned int)file_size);

    if (compressed_size <= 0) {
        printf("Huffman compression failed\n");
        free(input_data);
        free(compressed_data);
        return;
    }

    // Write compressed data to output file WITH ORIGINAL SIZE
    FILE* out_file = fopen(output_file, "wb");
    if (!out_file) {
        printf("Cannot open output file: %s\n", output_file);
        free(input_data);
        free(compressed_data);
        return;
    }

    // Write original file size first (8 bytes)
    fwrite(&file_size, sizeof(long), 1, out_file);
    // Then write compressed data
    fwrite(compressed_data, 1, (size_t)compressed_size, out_file);
    fclose(out_file);

    printf("Huffman compression completed:\n");
    printf("  Input: %s (%ld bytes)\n", input_file, file_size);
    printf("  Output: %s (%d bytes + 8 byte header)\n", output_file, compressed_size);
    printf("  Compression ratio: %.2f%%\n", (1.0 - (float)(compressed_size + 8) / file_size) * 100);

    free(input_data);
    free(compressed_data);
}

void huffman_decompress_file(const char* input_file, const char* output_file) {
    FILE* in_file = fopen(input_file, "rb");
    if (!in_file) {
        printf("Cannot open input file: %s\n", input_file);
        return;
    }

    // Read original file size from header
    long original_size;
    if (fread(&original_size, sizeof(long), 1, in_file) != 1) {
        printf("Error reading original size from compressed file\n");
        fclose(in_file);
        return;
    }

    // Get remaining file size (compressed data)
    fseek(in_file, 0, SEEK_END);
    long compressed_data_size = ftell(in_file) - sizeof(long);
    fseek(in_file, sizeof(long), SEEK_SET); // Skip header

    if (compressed_data_size <= 0) {
        printf("Invalid compressed data size\n");
        fclose(in_file);
        return;
    }

    // Read compressed data
    unsigned char* compressed_data = (unsigned char*)malloc((size_t)compressed_data_size);
    if (!compressed_data) {
        printf("Memory allocation failed for compressed data\n");
        fclose(in_file);
        return;
    }

    size_t bytes_read = fread(compressed_data, 1, (size_t)compressed_data_size, in_file);
    fclose(in_file);

    if (bytes_read != (size_t)compressed_data_size) {
        printf("Error reading compressed file\n");
        free(compressed_data);
        return;
    }

    // Decompress data
    unsigned char* decompressed_data = (unsigned char*)malloc(original_size);
    if (!decompressed_data) {
        printf("Memory allocation failed for decompressed data\n");
        free(compressed_data);
        return;
    }

    Huffman_Uncompress(compressed_data, decompressed_data, (unsigned int)compressed_data_size, (unsigned int)original_size);

    // Write decompressed data to output file
    FILE* out_file = fopen(output_file, "wb");
    if (!out_file) {
        printf("Cannot open output file: %s\n", output_file);
        free(compressed_data);
        free(decompressed_data);
        return;
    }

    fwrite(decompressed_data, 1, original_size, out_file);
    fclose(out_file);

    printf("Huffman decompression completed:\n");
    printf("  Input: %s (%ld bytes compressed + 8 byte header)\n", input_file, compressed_data_size);
    printf("  Output: %s (%ld bytes)\n", output_file, original_size);

    free(compressed_data);
    free(decompressed_data);
}

/*************************************************************************
*                           INTERNAL FUNCTIONS                           *
*************************************************************************/


/*************************************************************************
* _RLE_WriteRep() - Encode a repetition of 'symbol' repeated 'count'
* times.
*************************************************************************/

static void _RLE_WriteRep(unsigned char* out, unsigned int* outpos,
    unsigned char marker, unsigned char symbol, unsigned int count)
{
    unsigned int i, idx;

    idx = *outpos;
    if (count <= 3)
    {
        if (symbol == marker)
        {
            out[idx++] = marker;
            out[idx++] = count - 1;
        }
        else
        {
            for (i = 0; i < count; ++i)
            {
                out[idx++] = symbol;
            }
        }
    }
    else
    {
        out[idx++] = marker;
        --count;
        if (count >= 128)
        {
            out[idx++] = (count >> 8) | 0x80;
        }
        out[idx++] = count & 0xff;
        out[idx++] = symbol;
    }
    *outpos = idx;
}


/*************************************************************************
* _RLE_WriteNonRep() - Encode a non-repeating symbol, 'symbol'. 'marker'
* is the marker symbol, and special care has to be taken for the case
* when 'symbol' == 'marker'.
*************************************************************************/

static void _RLE_WriteNonRep(unsigned char* out, unsigned int* outpos,
    unsigned char marker, unsigned char symbol)
{
    unsigned int idx;

    idx = *outpos;
    if (symbol == marker)
    {
        out[idx++] = marker;
        out[idx++] = 0;
    }
    else
    {
        out[idx++] = symbol;
    }
    *outpos = idx;
}

/*************************************************************************
*                            PUBLIC FUNCTIONS                            *
*************************************************************************/

/*************************************************************************
* RLE_Compress() - Compress a block of data using an RLE coder.
*  in     - Input (uncompressed) buffer.
*  out    - Output (compressed) buffer. This buffer must be 0.4% larger
*           than the input buffer, plus one byte.
*  insize - Number of input bytes.
* The function returns the size of the compressed data.
*************************************************************************/

int RLE_Compress(unsigned char* in, unsigned char* out, unsigned int insize)
{
    unsigned char byte1, byte2, marker;
    unsigned int  inpos, outpos, count, i, histogram[256];

    /* Do we have anything to compress? */
    if (insize < 1)
    {
        return 0;
    }

    /* Create histogram */
    for (i = 0; i < 256; ++i)
    {
        histogram[i] = 0;
    }
    for (i = 0; i < insize; ++i)
    {
        ++histogram[in[i]];
    }

    /* Find the least common byte, and use it as the repetition marker */
    marker = 0;
    for (i = 1; i < 256; ++i)
    {
        if (histogram[i] < histogram[marker])
        {
            marker = i;
        }
    }

    /* Remember the repetition marker for the decoder */
    out[0] = marker;
    outpos = 1;

    /* Start of compression */
    byte1 = in[0];
    inpos = 1;
    count = 1;

    /* Are there at least two bytes? */
    if (insize >= 2)
    {
        byte2 = in[inpos++];
        count = 2;

        /* Main compression loop */
        do
        {
            if (byte1 == byte2)
            {
                /* Do we meet only a sequence of identical bytes? */
                while ((inpos < insize) && (byte1 == byte2) &&
                    (count < 32768))
                {
                    byte2 = in[inpos++];
                    ++count;
                }
                if (byte1 == byte2)
                {
                    _RLE_WriteRep(out, &outpos, marker, byte1, count);
                    if (inpos < insize)
                    {
                        byte1 = in[inpos++];
                        count = 1;
                    }
                    else
                    {
                        count = 0;
                    }
                }
                else
                {
                    _RLE_WriteRep(out, &outpos, marker, byte1, count - 1);
                    byte1 = byte2;
                    count = 1;
                }
            }
            else
            {
                /* No, then don't handle the last byte */
                _RLE_WriteNonRep(out, &outpos, marker, byte1);
                byte1 = byte2;
                count = 1;
            }
            if (inpos < insize)
            {
                byte2 = in[inpos++];
                count = 2;
            }
        } while ((inpos < insize) || (count >= 2));
    }

    /* One byte left? */
    if (count == 1)
    {
        _RLE_WriteNonRep(out, &outpos, marker, byte1);
    }

    return outpos;
}

/*************************************************************************
* RLE_Uncompress() - Uncompress a block of data using an RLE decoder.
*  in      - Input (compressed) buffer.
*  out     - Output (uncompressed) buffer. This buffer must be large
*            enough to hold the uncompressed data.
*  insize  - Number of input bytes.
*************************************************************************/

void RLE_Uncompress(unsigned char* in, unsigned char* out, unsigned int insize)
{
    unsigned char marker, symbol;
    unsigned int  i, inpos, outpos, count;

    /* Do we have anything to uncompress? */
    if (insize < 1)
    {
        return;
    }

    /* Get marker symbol from input stream */
    inpos = 0;
    marker = in[inpos++];

    /* Main decompression loop */
    outpos = 0;
    do
    {
        symbol = in[inpos++];
        if (symbol == marker)
        {
            /* We had a marker byte */
            count = in[inpos++];
            if (count <= 2)
            {
                /* Counts 0, 1 and 2 are used for marker byte repetition
                   only */
                for (i = 0; i <= count; ++i)
                {
                    out[outpos++] = marker;
                }
            }
            else
            {
                if (count & 0x80)
                {
                    count = ((count & 0x7f) << 8) + in[inpos++];
                }
                symbol = in[inpos++];
                for (i = 0; i <= count; ++i)
                {
                    out[outpos++] = symbol;
                }
            }
        }
        else
        {
            /* No marker, plain copy */
            out[outpos++] = symbol;
        }
    } while (inpos < insize);
}

/*************************************************************************
* New byte-oriented RLE functions with overflow protection
*************************************************************************/

int RLE_Encode(unsigned char* in, unsigned int inlen, unsigned char* out, unsigned int outlen, unsigned char esc) {
    if (inlen == 0) return 0;
    if (outlen == 0) return -1;

    unsigned int inpos = 0, outpos = 0;

    while (inpos < inlen) {
        unsigned char current = in[inpos];
        unsigned int count = 1;

        // Count consecutive identical bytes
        while (inpos + count < inlen && in[inpos + count] == current && count < 255) {
            count++;
        }

        // Check if we should use RLE encoding
        if (count > 3 || current == esc) {
            // Use RLE encoding - need 3 bytes in output
            if (outpos + 3 > outlen) {
                return -1; // Output buffer overflow
            }

            out[outpos++] = esc;
            out[outpos++] = (unsigned char)count;
            out[outpos++] = current;
            inpos += count;
        }
        else {
            // Copy literal bytes
            for (unsigned int i = 0; i < count; i++) {
                if (outpos >= outlen) {
                    return -1; // Output buffer overflow
                }
                out[outpos++] = current;
            }
            inpos += count;
        }
    }

    return (int)outpos;
}

int RLE_Decode(unsigned char* in, unsigned int inlen, unsigned char* out, unsigned int outlen, unsigned char esc) {
    if (inlen == 0) return 0;
    if (outlen == 0) return -1;

    unsigned int inpos = 0, outpos = 0;

    while (inpos < inlen && outpos < outlen) {
        unsigned char current = in[inpos++];

        if (current == esc && inpos < inlen) {
            // Potential RLE encoded sequence
            unsigned int count = in[inpos++];

            if (inpos < inlen) {
                unsigned char value = in[inpos++];

                // Check for output buffer overflow
                if (outpos + count > outlen) {
                    return -1; // Output buffer overflow
                }

                // Write the repeated bytes
                for (unsigned int i = 0; i < count; i++) {
                    out[outpos++] = value;
                }
            }
            else {
                // Malformed input - ESC at end without complete sequence
                if (outpos < outlen) {
                    out[outpos++] = current;
                }
                if (inpos - 1 < inlen && outpos < outlen) {
                    out[outpos++] = (unsigned char)count;
                }
            }
        }
        else {
            // Literal byte
            if (outpos < outlen) {
                out[outpos++] = current;
            }
            else {
                return -1; // Output buffer overflow
            }
        }
    }

    return (int)outpos;
}


// RLE file compression wrapper
void rle_compress_file(const char* input_file, const char* output_file) {
    FILE* in_file = fopen(input_file, "rb");
    if (!in_file) {
        printf("Cannot open input file: %s\n", input_file);
        return;
    }

    // Get file size
    fseek(in_file, 0, SEEK_END);
    long file_size = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    if (file_size <= 0) {
        printf("Empty file or error reading file size\n");
        fclose(in_file);
        return;
    }

    // Read input data
    unsigned char* input_data = (unsigned char*)malloc((size_t)file_size);
    if (!input_data) {
        printf("Memory allocation failed\n");
        fclose(in_file);
        return;
    }

    size_t bytes_read = fread(input_data, 1, (size_t)file_size, in_file);
    fclose(in_file);

    if (bytes_read != (size_t)file_size) {
        printf("Error reading input file\n");
        free(input_data);
        return;
    }

    // Compress data - RLE output buffer should be larger than input
    unsigned char* compressed_data = (unsigned char*)malloc((size_t)file_size * 2);
    if (!compressed_data) {
        printf("Memory allocation failed for compressed data\n");
        free(input_data);
        return;
    }

    int compressed_size = RLE_Encode(input_data, (unsigned int)file_size, compressed_data, (unsigned int)file_size * 2, 0x90);

    if (compressed_size <= 0) {
        printf("RLE compression failed\n");
        free(input_data);
        free(compressed_data);
        return;
    }

    // Write compressed data to output file
    FILE* out_file = fopen(output_file, "wb");
    if (!out_file) {
        printf("Cannot open output file: %s\n", output_file);
        free(input_data);
        free(compressed_data);
        return;
    }

    // Write original file size first
    fwrite(&file_size, sizeof(long), 1, out_file);
    // Then write compressed data
    fwrite(compressed_data, 1, (size_t)compressed_size, out_file);
    fclose(out_file);

    printf("RLE compression completed:\n");
    printf("  Input: %s (%ld bytes)\n", input_file, file_size);
    printf("  Output: %s (%d bytes + 8 byte header)\n", output_file, compressed_size);
    printf("  Compression ratio: %.2f%%\n", (1.0 - (float)(compressed_size + 8) / file_size) * 100);

    free(input_data);
    free(compressed_data);
}

// RLE file decompression wrapper
void rle_decompress_file(const char* input_file, const char* output_file) {
    FILE* in_file = fopen(input_file, "rb");
    if (!in_file) {
        printf("Cannot open input file: %s\n", input_file);
        return;
    }

    // Read original file size from header
    long original_size;
    if (fread(&original_size, sizeof(long), 1, in_file) != 1) {
        printf("Error reading original size from compressed file\n");
        fclose(in_file);
        return;
    }

    // Get remaining file size (compressed data)
    fseek(in_file, 0, SEEK_END);
    long compressed_data_size = ftell(in_file) - sizeof(long);
    fseek(in_file, sizeof(long), SEEK_SET); // Skip header

    if (compressed_data_size <= 0) {
        printf("Invalid compressed data size\n");
        fclose(in_file);
        return;
    }

    // Read compressed data
    unsigned char* compressed_data = (unsigned char*)malloc((size_t)compressed_data_size);
    if (!compressed_data) {
        printf("Memory allocation failed for compressed data\n");
        fclose(in_file);
        return;
    }

    size_t bytes_read = fread(compressed_data, 1, (size_t)compressed_data_size, in_file);
    fclose(in_file);

    if (bytes_read != (size_t)compressed_data_size) {
        printf("Error reading compressed file\n");
        free(compressed_data);
        return;
    }

    // Decompress data
    unsigned char* decompressed_data = (unsigned char*)malloc(original_size);
    if (!decompressed_data) {
        printf("Memory allocation failed for decompressed data\n");
        free(compressed_data);
        return;
    }

    int result = RLE_Decode(compressed_data, (unsigned int)compressed_data_size, decompressed_data, (unsigned int)original_size, 0x90);

    if (result <= 0) {
        printf("RLE decompression failed\n");
        free(compressed_data);
        free(decompressed_data);
        return;
    }

    // Write decompressed data to output file
    FILE* out_file = fopen(output_file, "wb");
    if (!out_file) {
        printf("Cannot open output file: %s\n", output_file);
        free(compressed_data);
        free(decompressed_data);
        return;
    }

    fwrite(decompressed_data, 1, original_size, out_file);
    fclose(out_file);

    printf("RLE decompression completed:\n");
    printf("  Input: %s (%ld bytes compressed + 8 byte header)\n", input_file, compressed_data_size);
    printf("  Output: %s (%ld bytes)\n", output_file, original_size);

    free(compressed_data);
    free(decompressed_data);
}