//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include <Parser/Entry.h>
#include <Parser/Parser.h>
#include <Parser/TDRparser.h>

#include <catch.hpp>

TEST_CASE("Test of the TDR parser", "[TDRparser]")
{
    Parser::TDRparser parser;

    Parser::Parser::Status status;
    Parser::Entry_t entry = parser.GetEntry(status);
    REQUIRE( status == Parser::Parser::Status::ERROR );
}