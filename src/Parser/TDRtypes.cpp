//
// Created by Vetle Wegner Ingeberg on 16/12/2019.
//

#include "Parser/TDRtypes.h"

using namespace Parser;

/*bool Parser::operator==(const TDR_entry &lhs, const TDR_entry &rhs)
{
    bool req = (lhs.address == rhs.address);
    req = req & (lhs.timestamp == rhs.timestamp);
    req = req & (lhs.is_tdc != rhs.is_tdc);
    return req;
}*/

std::ostream &Parser::operator<<(std::ostream &str, const TDR_event_type_t &event)
{
    str << "\t\tTimestamp: " << event.timestamp << "\n";
    str << "\t\tADC_data: " << event.ADC_data << "\n";
    str << "\t\tchanID: " << event.chanID << "\n";
    str << "\t\tveto: ";
    if ( event.veto )
        str << "true";
    else
        str << "false";
    str << "\n";
    str << "\t\tfail: ";
    if ( event.fail )
        str << "true";
    else
        str << "false";
    str << "\n";
    str << "\t\tiden: ";
    switch ( event.ident ){
        case unknown :
            str << "unknown";
            break;
        case sample_trace :
            str << "sample_trace";
            break;
        case module_info :
            str << "module_info";
            break;
        case ADC_event :
            str << "ADC_event";
            break;
        default :
            str << "unknown";
            break;
    }
    return str;
}

std::ostream &Parser::operator<<(std::ostream &str, const TDR_explicit &entry)
{
    str << "\tTimestamp: " << "\t" << entry.timestamp << "\n";
    str << "\tAddress: " << "\t" << entry.address << "\n";
    str << "\tIs_TDC: " << "\t";
    if ( entry.is_tdc )
        str <<  "true";
    else
        str << "false";
    str << "\n";
    str << "\tRaw event:\n" << entry.evt;
    return str;
}
