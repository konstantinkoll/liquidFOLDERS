
// CWhiteButton.h: Schnittstelle der Klasse CWhiteButton
//

#pragma once
#include "CHoverButton.h"


// CWhiteButton
//

class CWhiteButton : public CHoverButton
{
public:
	CWhiteButton();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
