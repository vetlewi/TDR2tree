//
// Created by Vetle Wegner Ingeberg on 21/10/2019.
//

#include <iostream>

#include <Buffer/Buffer.h>
#include <Buffer/FileBufferFetcher.h>
#include <Buffer/STFileBufferFetcher.h>
#include <Buffer/MTFileBufferFetcher.h>

#include <Parser/Entry.h>
#include <Parser/TDRparser.h>

void ReadFile(const char *filename)
{
    Fetcher::FileBufferFetcher *bf = new Fetcher::MTFileBufferFetcher(new Fetcher::TDRBuffer);
    Parser::TDRparser parser;

    Fetcher::BufferFetcher::Status status = bf->Open(filename, 0);
    Parser::Parser::Status pstatus;
    const Fetcher::Buffer *buf;

    std::vector<Parser::Entry_t> entries;
    while ( status == Fetcher::BufferFetcher::OKAY ){
        buf = bf->Next(status);
        entries = parser.GetEntry(buf, pstatus);
    }


    delete bf;
}


int main()
{
    ReadFile("/Users/vetlewi/Desktop/R77_9");
    return 0;
}
