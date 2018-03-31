. This program reads +ve integers stored at location ARRY, counts the number of
. even and odd integers and stores them at locations EVEN and ODD respectively
EveOdd              START    0

FIRST               CLEAR    X
					CLEAR    S
					CLEAR    T

					LDB      #30

LOOP                LDA      ARRY,X
					AND 	 #1
					COMP	 #0
					LDA      #1
                    JEQ      EVENNUM
                    ADDR	 A,S
                    J 		 MID
EVENNUM             ADDR     A,T
MID                 LDA      #3
					ADDR	 A,X
					COMPR	 X,B
					JLT		 LOOP

					STS      ODD 				STORE number of odd integers at ODD location 
					STT      EVEN 				STORE number of even integers at EVEN location

					RSUB 						terminate the program and RETURN the control to OS

. The numbers are stored at location ARRY
ARRY                BYTE     X'79A0B1126A703680C51273902AAB10'
ARRY1				BYTE	 X'40AC0955608A0C7D0B7B0D000000AF'
EVEN                RESW	 10
ODD                 RESB     30
                    END      FIRST
