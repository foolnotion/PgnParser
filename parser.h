#ifndef PARSER_H
#define PARSER_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <string>
#include <stdexcept>
#include <iostream>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

struct PgnDatabase {
    PgnDatabase() {}
    PgnDatabase(const std::string& s) : str(s) {}

    std::string str;
};

template <typename Iterator, typename Skipper>
struct PgnParser : qi::grammar<Iterator, PgnDatabase(), Skipper> {
    PgnParser() : PgnParser::base_type(start) {
        start %= pgn_database;
        pgn_database = *pgn_game;
        pgn_game = tag_section >> movetext_section;

        movetext_section = element_sequence >> game_result;

        element_sequence = *(element | recursive_variation);

        element = move_no
                  >> +san_move
                  >> -comment
                  >> -nag;

        recursive_variation = '(' >> element_sequence >> ')';

        move_no = qi::uint_ >> '.';
        symbol = qi::char_("KQRBNPkqrbnp");
        capture = qi::char_("x");
        column_id = +qi::char_("A-Ha-h");
        line_id = qi::char_("1-8");

        san_move = (piece_move | pawn_move | castle) >> -check;
        piece_move = -symbol >> -capture >> column_id >> line_id; // example: Nxe4 (Knight takes on e4)
        pawn_move = column_id >> -(capture >> column_id) >> line_id;
        castle = qi::string("O-O") | qi::string("O-O-O");
        check = qi::char_('+');

        game_result = qi::string("1-0")
                      | qi::string("0-1")
                      | qi::string("1/2-1/2")
                      | qi::string("*");

        tag_section = *tag_pair;
        tag_pair = '[' >> tag_name >> tag_value >> ']'; // example: [White "Fischer, Robert J."]

        tag_name = qi::string("Event")
                   | qi::string("Site")
                   | qi::string("Date")
                   | qi::string("Round")
                   | qi::string("White")
                   | qi::string("Black")
                   | qi::string("Result");

        tag_value = '"' >> text_value >> '"';
        comment = '{' >> text_value >> '}';
        text_value = qi::lexeme[*(qi::char_("A-Za-z0-9")
                                  | qi::char_("-_/.,|\\")
                                  | qi::space)];

        nag = '$' >> qi::int_;

        pgn_database.name("pgn_database");
        pgn_game.name("pgn_game");
        movetext_section.name("movetext_section");
        element_sequence.name("element_sequence");
        recursive_variation.name("recursive_variation");
        move_no.name("move_number");
        san_move.name("san_move");
        game_result.name("game_termination");
        tag_section.name("tag_section");
        tag_pair.name("tag_pair");
        tag_name.name("tag_name");
        tag_value.name("tag_value");
        comment.name("comment");
        column_id.name("column_identifier");
        line_id.name("line_identifier");
        symbol.name("symbol");
        nag.name("nag");
    }

    qi::rule<Iterator, std::string(), Skipper>
            pgn_database, pgn_game, tag_section, tag_pair, tag_name, tag_value, text_value,
            comment, movetext_section, element_sequence, element, recursive_variation,
            game_result, empty, move_no, san_move, piece_move, pawn_move, castle, check,
            symbol, column_id, line_id, capture, nag;

    qi::rule<Iterator, PgnDatabase(), Skipper> start;

};

#endif // PARSER_H
