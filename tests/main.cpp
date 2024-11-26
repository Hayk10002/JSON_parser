#include <iostream>

#include "json_parser.hpp"

using namespace hayk10002;

int main()
{
    Json value = Json::object(
        {
            {"name", Json::boolean(true)}, 
            {"age", Json::string("hehe")}
        });

    //auto x = { 1, 4.5f, "343"};

    return 0;
}