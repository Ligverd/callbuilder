/***************************************************************************
 *   Copyright (C) 2007 by PAX   *
 *   pax@m-200.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <CDRBuilder.h>

class CDialog
{
	static char ConvertTable(const char c);
public:
	static void CDialog::RemoveFiles(CParser &Parser);
	static bool WriteStringsToFile(const char* sDir, const char* sFileName, CDRBuilding::TPCharList lst);
	static void ErrorActions(CParser& Parser, const CDRBuilding::CCDRBuilder& builder, char* sMessage);
	static int CDialog::SaveResiduaryCalls(const char* sFileName, const CDRBuilding::CCDRBuilder& builder);
	static int CDialog::PutResiduaryCalls(const char* sFileName, CDRBuilding::CCDRBuilder& builder);
	static void CDialog::RestoreMessage(int &fd_tfs, CParser &Parser, CDRBuilding::CCDRBuilder& builder, DWORD
	&iReadBytes, DWORD &dwLen, CDRBuilding::TPCharList &lstStr);
	static void CDialog::Visualisation(CParser &Parser);
	static int Converter(const char* sDir ,const char* sFileName);
	
};
