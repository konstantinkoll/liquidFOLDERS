
#pragma once

template<UINT Scale>
void HQScale24(const BITMAPINFO& DIB, LPBYTE pBitsSrc, LPBYTE pBitsDst)
{
	assert(DIB.bmiHeader.biWidth==128);

	for (LONG Row=0; Row<DIB.bmiHeader.biHeight; Row++)
	{
		for (LONG Column=0; Column<128; Column++)
		{
			// Accumulate colors
			UINT aBlue = 0;
			UINT aGreen = 0;
			UINT aRed = 0;

			for (LONG X=0; X<Scale; X++)
			{
				for (LONG Y=0; Y<Scale; Y++)
				{
					aBlue += *(pBitsSrc);
					aGreen += *(pBitsSrc+1);
					aRed += *(pBitsSrc+2);

					// Move source pointer to next row
					pBitsSrc += 128*3*Scale;
				}

				// Move source pointer four rows up and one column right
				pBitsSrc -= 3*(128*Scale*Scale-1);
			}

			// Divide by accumulated alpha
			*(pBitsDst+0) = (BYTE)(aBlue/(Scale*Scale));
			*(pBitsDst+1) = (BYTE)(aGreen/(Scale*Scale));
			*(pBitsDst+2) = (BYTE)(aRed/(Scale*Scale));

			// Move destination pointer one column right
			pBitsDst += 3;
		}

		// Move source pointer three rows down
		pBitsSrc += 128*3*Scale*(Scale-1);
	}
}

template<UINT Scale>
void HQScale32(const BITMAPINFO& DIB, LPBYTE pBitsSrc, LPBYTE pBitsDst)
{
	assert(DIB.bmiHeader.biWidth==128);

	for (LONG Row=0; Row<DIB.bmiHeader.biHeight; Row++)
	{
		for (LONG Column=0; Column<128; Column++)
		{
			// Accumulate colors
			UINT aBlue = 0;
			UINT aGreen = 0;
			UINT aRed = 0;
			UINT aAlpha = 0;

			for (LONG X=0; X<Scale; X++)
			{
				for (LONG Y=0; Y<Scale; Y++)
				{
					const UINT Alpha = *(pBitsSrc+3);

					aBlue += *(pBitsSrc)*Alpha;
					aGreen += *(pBitsSrc+1)*Alpha;
					aRed += *(pBitsSrc+2)*Alpha;
					aAlpha += Alpha;

					// Move source pointer to next row
					pBitsSrc += 128*4*Scale;
				}

				// Move source pointer four rows up and one column right
				pBitsSrc -= 4*(128*Scale*Scale-1);
			}

			// Divide by accumulated alpha and pre-multiply colors in one go
			*(pBitsDst+0) = (BYTE)(aBlue/(Scale*Scale*255));
			*(pBitsDst+1) = (BYTE)(aGreen/(Scale*Scale*255));
			*(pBitsDst+2) = (BYTE)(aRed/(Scale*Scale*255));
			*(pBitsDst+3) = (BYTE)(aAlpha/(Scale*Scale));

			// Move destination pointer one column right
			pBitsDst += 4;
		}

		// Move source pointer three rows down
		pBitsSrc += 128*4*Scale*(Scale-1);
	}
}
