#ifndef IMGDATA_H_
#define IMGDATA_H_

#define RES_X             16
#define RES_Z             32

unsigned char imgdata[RES_Z][RES_X/8] PROGMEM =
                          {
                        			{0b11011111, 0b10001110},
                        			{0b11011111, 0b10000010},
                        			{0b11011111, 0b10000010},
                        			{0b11011111, 0b10000011},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10000000},
                        			{0b11010011, 0b10001110},
                        			{0b11010011, 0b10000010},
                        			{0b11010011, 0b10000010},
                        			{0b11011111, 0b10001110},

                        			{0b11010011, 0b10001111},
                        			{0b11010011, 0b10001110},
                        			{0b11010011, 0b10001010},
                        			{0b11010011, 0b10001010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11010000, 0b10000010},
                        			{0b11011110, 0b10000010},
                        			{0b11010010, 0b10000010},
                        			{0b11010010, 0b10000010},
                        			{0b11010010, 0b10000010}
                          };

#endif /*IMGDATA_H_*/
