/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 6 2025
Comments:		Projects III - Coded Messaging System

				Config header file

==========================================================================================================================
*/
#pragma once

void saveConfig(int tx, int rx, int headers, int encrypt, int compress, int sid);
void loadConfig(int* tx, int* rx, int* headers, int* encrypt, int* compress, int* sid);
