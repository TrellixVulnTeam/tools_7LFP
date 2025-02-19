/*********************************************************************
Copyright 2013 Karl Jones
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

For Further Information Please Contact me at
Karljj1@yahoo.com
http://p.sf.net/kdis/UserGuide
*********************************************************************/

#include "./PDU_FactoryLink16.h"

#include "./../PDU/Link16/Link16_Signal_PDU.h"
#include "./../PDU/Link16/Link16_Transmitter_PDU.h"

using namespace std;
using namespace KDIS;
using namespace DATA_TYPE;
using namespace UTILS;
using namespace PDU;
using namespace PDU::LINK16;
using namespace ENUMS;

//////////////////////////////////////////////////////////////////////////
// public:
//////////////////////////////////////////////////////////////////////////

PDU_FactoryLink16::PDU_FactoryLink16()
{
}

//////////////////////////////////////////////////////////////////////////

PDU_FactoryLink16::~PDU_FactoryLink16()
{
}

//////////////////////////////////////////////////////////////////////////

auto_ptr<Header> PDU_FactoryLink16::Decode( const Header & H, KDataStream & Stream )throw( KException )
{
    switch( H.GetPDUType() )
    {
	case Signal_PDU_Type:
        return applyFilters( new Link16_Signal_PDU( H, Stream ) );    

	case Transmitter_PDU_Type:
        return applyFilters( new Link16_Transmitter_PDU( H, Stream ) );   
    }

    // Its not a Link 16 PDU, try the default.
	return PDU_Factory::Decode( H, Stream );    
}

//////////////////////////////////////////////////////////////////////////
