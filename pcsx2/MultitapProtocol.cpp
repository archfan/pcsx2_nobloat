/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2022  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "MultitapProtocol.h"
#include "Sio.h"

#define MT_LOG_ENABLE 0
#define MT_LOG if (MT_LOG_ENABLE) DevCon

MultitapProtocol g_MultitapProtocol;

void MultitapProtocol::SupportCheck()
{
	MT_LOG.WriteLn("%s", __FUNCTION__);
	fifoOut.push_back(0x5a);
	fifoOut.push_back(0x04);
	fifoOut.push_back(0x00);
	fifoOut.push_back(0x5a);
}

void MultitapProtocol::Select()
{
	MT_LOG.WriteLn("%s", __FUNCTION__);
	const u8 newSlot = fifoIn.front();
	fifoIn.pop_front();
	const bool isInBounds = (newSlot < SIO::SLOTS);

	if (isInBounds)
	{
		sio2.slot = newSlot;
		MT_LOG.WriteLn("Slot changed to %d", sio2.slot);
	}

	fifoOut.push_back(0x5a);
	fifoOut.push_back(0x00);
	fifoOut.push_back(0x00);
	fifoOut.push_back(isInBounds ? newSlot : 0xff);
	fifoOut.push_back(isInBounds ? 0x5a : 0x66);
}

MultitapProtocol::MultitapProtocol() = default;
MultitapProtocol::~MultitapProtocol() = default;

void MultitapProtocol::SoftReset()
{
}

void MultitapProtocol::FullReset()
{
	SoftReset();

	sio2.slot = 0;
}

void MultitapProtocol::SendToMultitap()
{
	const u8 commandByte = fifoIn.front();
	fifoIn.pop_front();
	fifoOut.push_back(0x80);

	switch (static_cast<MultitapMode>(commandByte))
	{
		case MultitapMode::PAD_SUPPORT_CHECK:
		case MultitapMode::MEMCARD_SUPPORT_CHECK:
			SupportCheck();
			break;
		case MultitapMode::SELECT_PAD:
		case MultitapMode::SELECT_MEMCARD:
			Select();
			break;
		default:
			DevCon.Warning("%s() Unhandled MultitapMode (%02X)", __FUNCTION__, commandByte);
			break;
	}
}
