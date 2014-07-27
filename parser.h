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

void print(std::string const& s) {
	std::cout << s << std::endl;
}

template <typename Iterator, typename Skipper>
struct PgnParser : qi::grammar<Iterator, PgnDatabase(), Skipper> {
    PgnParser() : PgnParser::base_type(start) {
        start %= pgn_database;
        pgn_database = *pgn_game;
        pgn_game = tag_section >> movetext_section;

        movetext_section = element_sequence >> game_result;

        element_sequence = *(element | recursive_variation);

		// comments can appear before the move number or before the move or between moves or even after
        element = -comment 
				  >> move_no
				  >> -comment
                  >> +(san_move >> -nag >> -comment);
		          
        recursive_variation = '(' >> element_sequence >> ')';

        san_move = (piece_move | pawn_move | castle | null_move) >> -check >> -mate;

        piece_move = symbol >> (full_move_desc | partial_move_desc); 

		full_move_desc = (column_id | line_id) >> -capture >> square; // example Nbd7, Nfxd5 (piece symbol not included)
		partial_move_desc = (-capture >> square); // Nf3, Nc6, Bb5 etc (piece symbol not included)

        pawn_move = column_id >> -(capture >> column_id) >> line_id;

        symbol = qi::char_("KQRBNkqrbn");
        column_id = qi::char_("A-Ha-h");
        line_id = qi::char_("1-8");
		square = column_id >> line_id;
        castle = qi::string("O-O") | qi::string("O-O-O");
        move_no = qi::uint_ >> '.';
        capture = qi::char_('x');
        check = qi::char_('+');
		mate = qi::char_('#');
		null_move = qi::string("..");

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
                                  | qi::char_("-_/.,|\\?!*:")
                                  | qi::space)];

        nag = '$' >> qi::int_;

        pgn_database.name("pgn_database");
        pgn_game.name("pgn_game");
        movetext_section.name("movetext_section");
        element_sequence.name("element_sequence");
		element.name("element");
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

		//qi::debug(comment);
		//qi::debug(element);
    }

    qi::rule<Iterator, std::string(), Skipper>
            pgn_database, pgn_game, tag_section, tag_pair, tag_name, tag_value, text_value,
            comment, movetext_section, element_sequence, element, recursive_variation,
            game_result, empty, move_no, san_move, piece_move, pawn_move, castle, null_move, 
            full_move_desc, partial_move_desc, check, mate, symbol, column_id, line_id, square, capture, nag;

    qi::rule<Iterator, PgnDatabase(), Skipper> start;

};

#endif // PARSER_H
