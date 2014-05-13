/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <sstream>
#include <server/nautical/NavNmea.h>
#include <server/common/string.h>

using namespace sail;

namespace {
  const char data001[] = "$IIRMB,A,1.00,R,,VIDY,,,,,003.16,281,,V,A*65\n$IIRMB,A,2.00,R,,VIDY,,,,,003.16,281,,V,A*66\n$IIRMB,A,3.00,R,,VIDY,,,,,003.16,281,,V,A*67\n$IIRMB,A,4.00,R,,VIDY,,,,,003.16,281,,V,A*60\n$IIRMB,A,5.00,R,,VIDY,,,,,003.16,281,,V,A*61\n$IIRMB,A,6.00,R,,VIDY,,,,,003.16,281,,V,A*62\n$IIRMB,A,7.00,R,,VIDY,,,,,003.16,281,,V,A*63\n$IIMWV,225,R,01.6,N,A*4C\n$IIVHW,,,062,M,,,,*49\n$IIVWR,135,L,01.6,N,,,,*53\n$IIHDG,062,,,,*4B\n$IIMTW,+00.0,C*4E\n$IIMWV,255,R,05.7,N,A*4C\n$IIVHW,,,062,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,105,L,05.7,N,,,,*53\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,266,R,05.0,N,A*4C\n$IIMWV,255,T,05.7,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,094,L,05.0,N,,,,*53\n$IIRMB,A,8.00,R,,VIDY,,,,,003.16,281,,V,A*6C\n$IIMTW,+20.5,C*4E\n$IIMWV,263,R,05.4,N,A*4C\n$IIMWV,266,T,05.0,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,097,L,05.4,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,257,R,04.9,N,A*4C\n$IIMWV,263,T,05.4,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,103,L,04.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,258,R,05.1,N,A*4C\n$IIMWV,257,T,04.9,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,102,L,05.1,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIRMB,A,9.00,R,,VIDY,,,,,003.16,281,,V,A*6D\n$IIMTW,+20.5,C*4E\n$IIMWV,259,R,06.1,N,A*4C\n$IIMWV,258,T,05.1,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,101,L,06.1,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,257,R,05.6,N,A*4C\n$IIMWV,259,T,06.1,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,103,L,05.6,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,262,R,05.0,N,A*4C\n$IIMWV,257,T,05.6,N,A*4C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,098,L,05.0,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIRMB,A,0.00,R,,VIDY,,,,,003.16,281,,V,A*64\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,262,R,06.6,N,A*4C\n$IIMWV,262,T,05.0,N,A*4C\n$IIRMC,192604,A,5450.264,N,00931.382,E,00.2,004,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,098,L,06.6,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.382,E,192605,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,256,R,05.4,N,A*4C\n$IIMWV,262,T,06.6,N,A*4C\n$IIRMC,192605,A,5450.264,N,00931.382,E,00.2,013,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,103,L,05.0,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.382,E,192606,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,257,R,05.0,N,A*4C\n$IIMWV,256,T,05.4,N,A*4C\n$IIRMC,192606,A,5450.264,N,00931.382,E,00.3,022,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIRMB,A,1.00,R,,VIDY,,,,,003.16,281,,V,A*65\n$IIVWR,099,L,05.3,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.382,E,192606,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,261,R,05.3,N,A*4C\n$IIMWV,257,T,05.0,N,A*4C\n$IIRMC,192606,A,5450.264,N,00931.382,E,00.3,022,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,097,L,04.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.383,E,192608,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,263,R,04.9,N,A*4C\n$IIMWV,261,T,05.3,N,A*4C\n$IIRMC,192609,A,5450.264,N,00931.383,E,00.2,027,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,099,L,05.2,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.383,E,192609,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,261,R,05.2,N,A*4C\n$IIRMB,A,2.00,R,,VIDY,,,,,003.16,281,,V,A*66\n$IIRMC,192610,A,5450.264,N,00931.383,E,00.2,037,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,096,L,04.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.383,E,192610,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,261,R,05.6,N,A*4C\n$IIMWV,264,T,04.9,N,A*4C\n$IIRMC,192611,A,5450.264,N,00931.383,E,00.2,037,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,099,L,05.6,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.383,E,192611,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,270,R,05.9,N,A*4C\n$IIMWV,261,T,05.6,N,A*4C\n$IIRMC,192612,A,5450.264,N,00931.383,E,00.2,045,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,090,L,05.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.383,E,192612,A,A*47\n$IIRMB,A,3.00,R,,VIDY,,,,,003.16,281,,V,A*67\n$IIMTW,+21.0,C*4E\n$IIMWV,262,R,05.4,N,A*4C\n$IIMWV,270,T,05.9,N,A*4C\n$IIRMC,192613,A,5450.264,N,00931.383,E,00.3,041,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,098,L,05.4,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.382,E,192614,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,256,R,04.4,N,A*4C\n$IIMWV,262,T,05.4,N,A*4C\n$IIRMC,192614,A,5450.264,N,00931.382,E,00.4,035,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,104,L,04.4,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.382,E,192615,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,254,R,04.4,N,A*4C\n$IIMWV,256,T,04.4,N,A*4C\n$IIRMC,192615,A,5450.264,N,00931.382,E,00.3,025,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIRMB,A,4.00,R,,VIDY,,,,,003.16,281,,V,A*60\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.264,N,00931.381,E,192616,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,264,R,04.2,N,A*4C\n$IIMWV,254,T,04.4,N,A*4C\n$IIRMC,192616,A,5450.264,N,00931.381,E,00.6,020,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,086,L,04.0,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.263,N,00931.381,E,192617,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,274,R,04.0,N,A*4C\n$IIMWV,264,T,04.2,N,A*4C\n$IIRMC,192617,A,5450.263,N,00931.381,E,00.6,020,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,091,L,04.1,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.382,E,192618,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,269,R,04.1,N,A*4C\n$IIMWV,274,T,04.0,N,A*4C\n$IIRMB,A,5.00,R,,VIDY,,,,,003.16,281,,V,A*61\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,094,L,04.2,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.261,N,00931.382,E,192619,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,266,R,04.2,N,A*4C\n$IIMWV,266,T,04.2,N,A*4C\n$IIRMC,192620,A,5450.261,N,00931.382,E,00.4,000,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,093,L,04.3,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.261,N,00931.382,E,192620,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,271,R,04.3,N,A*4C\n$IIMWV,267,T,04.3,N,A*4C\n$IIRMC,192621,A,5450.260,N,00931.381,E,00.3,349,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,089,L,04.3,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.260,N,00931.381,E,192621,A,A*47\n$IIHDG,061,,,,*4B\n$IIRMB,A,6.00,R,,VIDY,,,,,003.16,281,,V,A*62\n$IIMTW,+21.0,C*4E\n$IIMWV,268,R,04.1,N,A*4C\n$IIMWV,271,T,04.3,N,A*4C\n$IIRMC,192622,A,5450.260,N,00931.381,E,00.3,349,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,092,L,04.1,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.260,N,00931.381,E,192622,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,271,R,04.0,N,A*4C\n$IIMWV,268,T,04.1,N,A*4C\n$IIRMC,192623,A,5450.261,N,00931.381,E,00.2,347,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,089,L,04.0,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.261,N,00931.381,E,192624,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+21.0,C*4E\n$IIMWV,278,R,04.1,N,A*4C\n$IIMWV,271,T,04.0,N,A*4C\n$IIRMC,192624,A,5450.261,N,00931.381,E,00.2,347,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,082,L,04.1,N,,,,*53\n$IIRMB,A,7.00,R,,VIDY,,,,,003.16,281,,V,A*63\n$IIGLL,5450.261,N,00931.381,E,192625,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,280,R,04.2,N,A*4C\n$IIMWV,278,T,04.1,N,A*4C\n$IIRMC,192625,A,5450.261,N,00931.381,E,00.2,347,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,078,L,04.3,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.261,N,00931.382,E,192626,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,282,R,04.3,N,A*4C\n$IIMWV,280,T,04.2,N,A*4C\n$IIRMC,192626,A,5450.261,N,00931.382,E,00.3,357,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,080,L,04.4,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.261,N,00931.382,E,192627,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,280,R,04.4,N,A*4C\n$IIMWV,282,T,04.3,N,A*4C\n$IIRMC,192627,A,5450.261,N,00931.382,E,00.2,358,310713,,,A*5C\n$IIRMB,A,8.00,R,,VIDY,,,,,003.16,281,,V,A*6C\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,079,L,04.7,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.382,E,192628,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,281,R,04.7,N,A*4C\n$IIMWV,280,T,04.4,N,A*4C\n$IIRMC,192629,A,5450.262,N,00931.382,E,00.4,358,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,071,L,04.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.382,E,192629,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,289,R,04.9,N,A*4C\n$IIMWV,289,T,04.9,N,A*4C\n$IIRMC,192630,A,5450.262,N,00931.382,E,00.3,359,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,064,L,04.9,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.382,E,192630,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIRMB,A,9.00,R,,VIDY,,,,,003.16,281,,V,A*6D\n$IIMWV,297,R,04.6,N,A*4C\n$IIMWV,296,T,04.9,N,A*4C\n$IIRMC,192631,A,5450.262,N,00931.381,E,00.3,359,310713,,,A*5C\n$IIVHW,,,061,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,063,L,04.6,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.381,E,192631,A,A*47\n$IIHDG,061,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,290,R,04.6,N,A*4C\n$IIMWV,297,T,04.6,N,A*4C\n$IIRMC,192632,A,5450.262,N,00931.382,E,00.3,359,310713,,,A*5C\n$IIVHW,,,060,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,070,L,04.6,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIGLL,5450.262,N,00931.382,E,192632,A,A*47\n$IIHDG,060,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,284,R,04.2,N,A*4C\n$IIMWV,290,T,04.6,N,A*4C\n$IIRMC,192633,A,5450.262,N,00931.382,E,00.2,000,310713,,,A*5C\n$IIVHW,,,060,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,076,L,04.2,N,,,,*53\n$IIDPT,002.7,-1.0,*40\n$IIRMB,A,0.00,R,,VIDY,,,,,003.16,281,,V,A*64\n$IIGLL,5450.263,N,00931.381,E,192634,A,A*47\n$IIHDG,060,,,,*4B\n$IIMTW,+20.5,C*4E\n$IIMWV,277,R,04.1,N,A*4C\n$IIMWV,284,T,04.2,N,A*4C\n$IIRMC,192634,A,5450.263,N,00931.381,E,00.3,358,310713,,,A*5C\n$IIVHW,,,060,M,00.0,N,,*49\n$IIVLW,01330,N,000.0,N*4D\n$IIVWR,083,L,04.1,N,,,,*53";
  std::stringstream testfile001(data001);
}

TEST(NavNmeaTest, TestComplete) {
  ParsedNavs navs = loadNavsFromNmea(testfile001, Nav::debuggingBoatId());
  EXPECT_TRUE(navs.complete());
  EXPECT_TRUE(navs.navs().hasData());
  EXPECT_GE(navs.navs().size(), 0); // Number of times RMC occurs in the string to be parsed
}

namespace {
  const char data002[] = "$IIMTW,+15.5,C*39\n$IIMWV,280,R,05.7,N,A*1B\n$IIMWV,248,T,05.9,N,A*17\n$IIVHW,,,153,M,03.3,N,,*63\n$IIVWR,080,L,05.7,N,,,,*75\n$IIDPT,007.4,-1.0,*43\n$IIHDG,153,,,,*50\n$IIMTW,+16.0,C*3F\n$IIMWV,281,R,05.7,N,A*1A\n$IIMWV,247,T,06.0,N,A*12\n$IIVHW,,,153,M,03.3,N,,*63\n$IIVWR,079,L,05.7,N,,,,*73\n$IIDPT,007.4,-1.0,*43\n$IIHDG,150,,,,*53\n$IIMTW,+16.0,C*3F\n$IIMWV,283,R,05.6,N,A*19\n$IIMWV,247,T,05.9,N,A*18\n$IIVHW,,,150,M,03.3,N,,*60\n$IIVWR,077,L,05.6,N,,,,*7C\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.939,N,00610.108,E,113704,A,A*5C\n$IIHDG,147,,,,*55\n$IIMTW,+15.5,C*39\n$IIMWV,283,R,05.4,N,A*1B\n$IIMWV,248,T,05.8,N,A*16\n$IIRMC,113704,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E\n$IIVHW,,,147,M,03.4,N,,*61\n$IIVWR,076,L,05.5,N,,,,*7E\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.939,N,00610.108,E,113704,A,A*5C\n$IIHDG,146,,,,*54\n$IIMTW,+15.5,C*39\n$IIMWV,284,R,05.5,N,A*1D\n$IIMWV,246,T,05.6,N,A*16\n$IIRMB,A,0.00,L,,OUC ,,,,,025.57,047,-01.3,V,A*39\n$IIRMC,113706,A,4612.938,N,00610.112,E,03.7,145,100708,,,A*47\n$IIVHW,,,144,M,03.4,N,,*62\n$IIVWR,073,L,06.0,N,,,,*7D\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.938,N,00610.112,E,113706,A,A*54\n$IIHDG,144,,,,*56\n$IIMTW,+15.5,C*39\n$IIMWV,287,R,06.0,N,A*18\n$IIMWV,253,T,05.9,N,A*1D\n$IIRMB,A,0.01,L,,OUC ,,,,,025.57,047,-00.8,V,A*32\n$IIRMC,113706,A,4612.938,N,00610.112,E,03.7,145,100708,,,A*47\n$IIVHW,,,142,M,03.4,N,,*64\n$IIVWR,071,L,05.8,N,,,,*74\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.938,N,00610.112,E,113706,A,A*54\n$IIHDG,144,,,,*56\n$IIMTW,+15.5,C*39\n$IIMWV,289,R,05.9,N,A*1C\n$IIMWV,254,T,05.7,N,A*14\n$IIRMC,113708,A,4612.937,N,00610.114,E,03.7,145,100708,,,A*40\n$IIVHW,,,144,M,03.5,N,,*63\n$IIVWR,071,L,05.9,N,,,,*75\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.937,N,00610.114,E,113708,A,A*53\n$IIHDG,146,,,,*54\n$IIMTW,+15.5,C*39\n$IIMWV,288,R,05.9,N,A*1D\n$IIMWV,254,T,05.8,N,A*1B\n$IIRMB,A,0.01,L,,OUC ,,,,,025.57,047,-00.4,V,A*3E\n$IIRMC,113708,A,4612.937,N,00610.114,E,03.7,145,100708,,,A*40\n$IIVHW,,,146,M,03.5,N,,*61\n$IIVWR,079,L,05.7,N,,,,*73\n$IIDPT,007.4,-1.0,*43\n$IIGLL,4612.932,N,00610.117,E,113712,A,A*5E\n$IIHDG,168,,,,*58\n$IIMTW,+16.0,C*3F\n$IIMWV,281,R,05.7,N,A*1A\n$IIMWV,247,T,05.9,N,A*18\n$IIRMB,A,0.01,L,,OUC ,,,,,025.57,047,-00.9,V,A*33\n$IIRMC,113712,A,4612.932,N,00610.117,E,04.0,173,100708,,,A*48\n$IIVHW,,,175,M,03.6,N,,*62\n$IIVWR,081,L,05.5,N,,,,*76\n$IIDPT,007.2,-1.0,*45\n$IIGLL,4612.932,N,00610.117,E,113712,A,A*5E\n$IIHDG,175,,,,*54\n$IIMTW,+16.0,C*3F\n$IIMWV,279,R,05.5,N,A*1F\n$IIMWV,242,T,06.1,N,A*16\n$IIRMB,A,0.01,L,,OUC ,,,,,025.57,047,-01.8,V,A*33\n$IIRMC,113714,A,4612.930,N,00610.117,E,04.0,178,100708,,,A*47\n$IIVHW,,,180,M,03.6,N,,*68\n$IIVWR,090,L,05.2,N,,,,*71\n$IIDPT,007.2,-1.0,*45";
  std::stringstream testfile002(data002);
}

TEST(NavNmeaTest, TestComplete2) {
  ParsedNavs navs = loadNavsFromNmea(testfile002, Nav::debuggingBoatId());
  EXPECT_TRUE(navs.complete());
  EXPECT_GE(navs.navs().size(), 0);
}


TEST(NavNmeaTest, TestIncomplete) {
  const char dataOneTimeStamp[] = "$IIMWV,248,T,05.8,N,A*16\n$IIRMC,113704,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E";
  std::stringstream testfileOneTimeStamp(dataOneTimeStamp);

  ParsedNavs navs = loadNavsFromNmea(testfileOneTimeStamp, Nav::debuggingBoatId());
  EXPECT_FALSE(navs.complete());
  EXPECT_EQ(navs.navs().size(), 0); // Because measurements preceding the first time stamp should be dropped: They could potentially be arbitrarily old.
}


TEST(NavNmeaTest, TestSkipDueToLongThreshold) {
  /*
   * Nmea data with 3 time-pos sentences.
   *
   * The first time-pos sentence should be dropped, because whatever
   * data preceding it (none in this case) could be arbitrarily old.
   *
   * The last time-pos sentence occurs more than 2 minutes after the time-pos sentence before.
   * Therefore, the time of the data collected in between cannot be assigned a sufficiently accurate time.
   */
  const char dataWithALongGap[] = "$IIRMC,113704,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E"
                         "$IIRMC,113804,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E"
                         "$IIRMC,114104,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E";
  std::stringstream testfileWithALongGap(dataWithALongGap);
  ParsedNavs navs = loadNavsFromNmea(testfileWithALongGap, Nav::debuggingBoatId());
  EXPECT_FALSE(navs.complete());
  EXPECT_EQ(navs.navs().size(), 1);
}


TEST(NavNmeaTest, TestIncludeLastTwo) {
  /*
   * Nmea with 3 time-pos sentences.
   *
   * The first sentence will be dropped, for the same reason as for 'dataWithALongGap'.
   *
   * The last time-pos sentence however occurs close in time to the sentence before and therefore,
   * the data associated with and the resulting Nav will not be dropped.
   */
  const char dataWithoutLongGap[] = "$IIRMC,113704,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E"
                                        "$IIRMC,113804,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E"
                                        "$IIRMC,113904,A,4612.939,N,00610.108,E,03.5,157,100708,,,A*4E";
  std::stringstream testfileWithoutLongGap(dataWithoutLongGap);
  ParsedNavs navs = loadNavsFromNmea(testfileWithoutLongGap, Nav::debuggingBoatId());
  EXPECT_FALSE(navs.complete());
  EXPECT_EQ(navs.navs().size(), 2);
}






