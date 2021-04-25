#include <iostream>
#include <fstream>
#include <string>
#include <vector>

enum class GrammarType {
	Type0,
	Type1,
	Type2,
	Type3,
	TypeNULL
};

struct Grammar {
	std::vector<std::string> nonTerminals,
							 terminals;

	std::vector<std::pair<std::string, std::string>> productionRules;

	std::string startingPoint;
}gram1, gram2;

void PrintGrammar(Grammar grammar) {
	std::cout << "\n\nNonterminals: ";
	for (std::vector<std::string>::iterator it = grammar.nonTerminals.begin(); it != grammar.nonTerminals.end(); it++) {
		std::cout << "\"" << *it << "\" ";
	}

	std::cout << "\nTerminals: ";
	for (std::vector<std::string>::iterator it = grammar.terminals.begin(); it != grammar.terminals.end(); it++) {
		std::cout << "\"" << *it << "\" ";
	}

	std::cout << "\nStarting point: " << grammar.startingPoint;

	std::cout << "\nProduction rules: ";
	for (std::vector<std::pair<std::string, std::string>>::iterator it = grammar.productionRules.begin(); it != grammar.productionRules.end(); it++) {
		std::cout << "\"" << (*it).first << " -> " << (*it).second << "\" ";
	}
}

bool IsType3(std::pair<std::string, std::string> format, std::vector<std::pair<std::string, std::string>> rules) {
	if (format.first.length() > 1) { // if there is more than one character on the left, it cannot be type 3 (or type 2)
		return false;
	}

	if (format.second.find('N') == std::string::npos) { // if we can't find any non-terminal on the right, then we only have terminals
		if (format.second.compare("E") == 0) { // if the right side consists of only the empty word...
			// check if the left side non-terminal exists in any other rule on the right-side
			for (std::vector<std::pair<std::string, std::string>>::iterator itRule = rules.begin(); itRule != rules.end(); itRule++) {
				if ((*itRule).second.find(format.first)) { // if we do find it, it is not of type 3
					return false;
				}
			}
		}
		
		return true; // otherwise, we're good
	}

	if (format.second.find_first_of('N') != format.second.find_last_of('N')) { // we have more than one nonterminal on the right hand side
		return false;
	}

	if (format.second.find_first_of('T') == 0 && format.second.find_last_of('T') == format.second.length() - 1) {
		return false; // if the first character is a terminal, and so is the last one, then we have terminals on both the left and right sides of a non-terminal
	}

	// check if we have both A -> Bp and A -> pB
	return true; // we passed all checks, our rule is of type N -> n * T N or N -> N T * n, where n >= 0
}

bool IsType2(std::pair<std::string, std::string> format) {
	return (format.first.length() == 1); // we need exactly one character on the left, if we have more, the rule is at most of type 1.
	// this is because the right-side can consist of any combination of non-terminals and terminals
}

bool IsType1(std::pair<std::string, std::string> format, std::pair<std::string, std::string> rule, std::vector<std::pair<std::string, std::string>> rules, std::string startingPoint) {
	if (format.first.length() > format.second.length()) { // if the left hand side has more non-terminals/terminals than the right side, it's type 0
		return false;
	}

	if (format.second.compare("E") == 0) { // if the right side consists of only the empty word
		if (rule.first.compare(startingPoint) != 0) { // check if the starting point can't be found in our rule on the left side
			return false; // this means that the non-terminal on the left side is not a starting point and therefore this can't be type 1
		} // otherwise...
		// check if the starting point appears in any other rule
		for (std::vector<std::pair<std::string, std::string>>::iterator itRule = rules.begin(); itRule != rules.end(); itRule++) {
			if ((*itRule).second.find(startingPoint)) {
				return false; // and say it is type 0 if it does
			}
		}
	}

	if (format.first.length() > 1) { // this means we have some context we need to care for
		int nonTerminalOffset = 0; // determine where our non-terminal is
		for (std::string::iterator itFirst = rule.first.begin(); itFirst != rule.first.end(); itFirst++, nonTerminalOffset++) {
			if (rule.second.find(*itFirst) == std::string::npos) { // if we can't find the non-terminal on the right side, that is the non-terminal we replace
				break;
			}
			else if (rule.first.find_first_of(*itFirst) != rule.first.find_last_of(*itFirst)) {
				// this means we have two different instances of the same non-terminal
				
				int occurences = 0;
				for (std::string::iterator itStr = rule.first.begin(); itStr != rule.first.end(); itStr++) {
					if (*itFirst == *itStr) {
						occurences++;
					}
				}

				for (std::string::iterator itStr = rule.second.begin(); itStr != rule.second.end(); itStr++) {
					if (*itFirst == *itStr) {
						occurences--;
					}
				}

				if (occurences != 0) { // this means one of the non-terminals is the one we're looking for
					// check prefix for every non-terminal of that kind, the first prefix to be different is our left context
					size_t offset = rule.first.find(*itFirst), prevOffset = 0; 
					bool found = false;
					while (offset != std::string::npos) {
						if (rule.first.compare(prevOffset, offset, rule.second, prevOffset, offset) != 0) {
							found = true;
							nonTerminalOffset = prevOffset;
						}

						prevOffset = offset;
						offset = rule.first.find(*itFirst, offset + 1);
					}

					if (!found) {
						nonTerminalOffset = rule.first.size() - 1;
					}
				}
				break;
			}
		}

		std::string leftContext, rightContext;

		for (std::string::iterator itFirst = rule.first.begin(); itFirst != rule.first.end(); itFirst++) {
			if (itFirst - rule.first.begin() < nonTerminalOffset) {
				leftContext.push_back(*itFirst); // this is what's on the left of our non-terminal on the left side of our rule
			}
			else if (itFirst - rule.first.begin() > nonTerminalOffset){
				rightContext.push_back(*itFirst); // this is what's on the right of our non-terminal on the left side of our rule
			}
		}

		if (leftContext.length() > 0) { // if we have anything on the left of our non-terminal on the left side of the rule
			if (rule.second.compare(0, nonTerminalOffset, leftContext) != 0) { // and if the same thing can't be found on the right side of our rule
				return false; // then it's type 0
			}
		}

		if (rightContext.length() > 0) { // if we have anything on the right of our non-terminal on the left side of the rule
			if (rule.second.compare(nonTerminalOffset + 1, std::string::npos, rightContext) != 0) { // and if the same thing can't be found on the right side of our rule
				return false; // then it's type 0
			}
		}
	}

	return true; // we passed all checks, this is a type 1 rule.
}

GrammarType FindGrammarType(Grammar gram) {
	GrammarType type = gram.productionRules.empty() ? GrammarType::TypeNULL : GrammarType::Type3;
	
	for (std::vector<std::pair<std::string, std::string>>::iterator itRule = gram.productionRules.begin(); itRule != gram.productionRules.end() && type != GrammarType::Type0; itRule++) {
		std::pair<std::string, std::string> format; // N - non-terminal, T - terminal, E - empty word

		for (std::string::iterator itFirst = (*itRule).first.begin(); itFirst != (*itRule).first.end(); itFirst++) {
			std::string currStr; currStr.push_back(*itFirst);
			
			if (currStr.compare("|") == 0) {
				format.first = 'E';
				continue;
			}

			bool found = false;
			do {
				for (std::vector<std::string>::iterator itNT = gram.nonTerminals.begin(); itNT != gram.nonTerminals.end() && !found; itNT++) {
					if ((*itNT).compare(currStr) == 0) {
						format.first.push_back('N');
						found = true;
					}
				}

				for (std::vector<std::string>::iterator itT = gram.terminals.begin(); itT != gram.terminals.end() && !found; itT++) {
					if ((*itT).compare(currStr) == 0) {
						format.first.push_back('T');
						found = true;
					}
				}

				if (!found) {
					itFirst++;
					currStr.push_back(*itFirst);
				}
			} while (!found);
		}

		for (std::string::iterator itSecond = (*itRule).second.begin(); itSecond != (*itRule).second.end(); itSecond++) {
			std::string currStr; currStr.push_back(*itSecond);

			if (currStr.compare("|") == 0) {
				format.second = 'E';
				continue;
			}

			bool found = false;
			do {
				for (std::vector<std::string>::iterator itNT = gram.nonTerminals.begin(); itNT != gram.nonTerminals.end() && !found; itNT++) {
					if ((*itNT).compare(currStr) == 0) {
						format.second.push_back('N');
						found = true;
					}
				}

				for (std::vector<std::string>::iterator itT = gram.terminals.begin(); itT != gram.terminals.end() && !found; itT++) {
					if ((*itT).compare(currStr) == 0) {
						format.second.push_back('T');
						found = true;
					}
				}

				if (!found) {
					itSecond++;
					currStr.push_back(*itSecond);
				}
			} while (!found);
		}

		if (type == GrammarType::Type3) {
			if (!IsType3(format, gram.productionRules)) {
				type = GrammarType::Type2;
			}
		}

		if (type == GrammarType::Type2) {
			if (!IsType2(format)) {
				type = GrammarType::Type1;
			}
		}

		if (type == GrammarType::Type1) {
			if (!IsType1(format, *itRule, gram.productionRules, gram.startingPoint)) {
				type = GrammarType::Type0;
			}
		}
	}

	return type;
}

// Functions used across all closure properties

void AddTerminals(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::string>::iterator itO = otherGram.terminals.begin(); itO != otherGram.terminals.end(); itO++) {
		bool alreadyFound = false;
		for (std::vector<std::string>::iterator itN = newGram.terminals.begin(); itN != newGram.terminals.end() && !alreadyFound; itN++) {
			if (*itN == *itO) {
				alreadyFound = true;
			}
		}

		if (!alreadyFound) {
			newGram.terminals.push_back(*itO); // we just add all of them without worry
		}
	}
}

// Union functions
void AddNonTerminalsUnion(Grammar& newGram, Grammar& otherGram, std::vector<std::string> terminals) { 
	// adds the non-terminals and the starting point, making the necessary adjustments if needed
	if (newGram.nonTerminals.empty()) { // if we don't have any other non-terminals
		for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
			bool alreadyExists = false;
			do {
				alreadyExists = false;
				std::string foundString;
				for (std::vector<std::string>::iterator itT = terminals.begin(); itT != terminals.end() && !alreadyExists; itT++) {
					if ((*itT).compare(*itO) == 0) { // if the terminal already exists in the grammar we are trying to make
						alreadyExists = true;
						foundString = *itO;

						(*itO).push_back('\'');
					}
				}

				if (alreadyExists) {
					for (std::vector<std::pair<std::string, std::string>>::iterator itPair = otherGram.productionRules.begin(); itPair != otherGram.productionRules.end(); itPair++) {
						const size_t offsetFirst = (*itPair).first.find(foundString);
						if (offsetFirst != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetFirst; i++) {
								newStr.push_back((*itPair).first[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetFirst + 1; i < (*itPair).first.size(); i++) {
								newStr.push_back((*itPair).first[i]);
							}
							(*itPair).first = newStr;
						}

						const size_t offsetSecond = (*itPair).second.find(foundString);
						if (offsetSecond != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetSecond; i++) {
								newStr.push_back((*itPair).second[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetSecond + 1; i < (*itPair).second.size(); i++) {
								newStr.push_back((*itPair).second[i]);
							}
							(*itPair).second = newStr;
						}
					}
				}
			} while (alreadyExists);
			newGram.nonTerminals.push_back(*itO);
		}

		newGram.startingPoint.push_back('S'); // we make a new starting point

		bool alreadyExists;
		do {
			alreadyExists = false;
			for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
				if ((*itN).compare(newGram.startingPoint) == 0) { // we check if our new starting point already exists in our nonTerminals
					alreadyExists = true;
				}
			}

			if (alreadyExists) { // if it does already exist
				newGram.startingPoint.push_back('\''); // then add another ' to hopefully differentiate it from the others
			}
		} while (alreadyExists);

		newGram.nonTerminals.push_back(newGram.startingPoint); // finally, we add the starting point to our list of non-terminals
	}
	else {
		for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
			bool alreadyExists = false;
			do {
				alreadyExists = false;
				std::string foundString;
				for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
					if ((*itN).compare(*itO) == 0) { // if the non-terminal already exists in the grammar we are trying to make
						alreadyExists = true;
						foundString = *itO;

						(*itO).push_back('\'');
					}
				}

				if (!alreadyExists) {
					for (std::vector<std::string>::iterator itT = newGram.terminals.begin(); itT != newGram.terminals.end() && !alreadyExists; itT++) {
						if ((*itT).compare(*itO) == 0) { // if the terminal already exists in the grammar we are trying to make
							alreadyExists = true;
							foundString = *itO;

							(*itO).push_back('\'');
						}
					}

					if (foundString.compare(otherGram.startingPoint) == 0) {
						otherGram.startingPoint = *itO;
					}
				}

				if (alreadyExists) {
					for (std::vector<std::pair<std::string, std::string>>::iterator itPair = otherGram.productionRules.begin(); itPair != otherGram.productionRules.end(); itPair++) {
						const size_t offsetFirst = (*itPair).first.find(foundString);
						if (offsetFirst != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetFirst; i++) {
								newStr.push_back((*itPair).first[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetFirst + 1; i < (*itPair).first.size(); i++) {
								newStr.push_back((*itPair).first[i]);
							}
							(*itPair).first = newStr;
						}

						const size_t offsetSecond = (*itPair).second.find(foundString);
						if (offsetSecond != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetSecond; i++) {
								newStr.push_back((*itPair).second[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetSecond + 1; i < (*itPair).second.size(); i++) {
								newStr.push_back((*itPair).second[i]);
							}
							(*itPair).second = newStr;
						}
					}

					if (foundString.compare(otherGram.startingPoint) == 0) {
						otherGram.startingPoint = *itO;
					}
				}
			} while (alreadyExists);
			newGram.nonTerminals.push_back(*itO);
		}
	}
}

void AddProductionRulesUnion(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = otherGram.productionRules.begin(); itO != otherGram.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}

	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, otherGram.startingPoint));
}

Grammar CreateGrammarFromUnion(Grammar gram1, Grammar gram2) {
	Grammar newGram;

	AddNonTerminalsUnion(newGram, gram1, gram2.terminals);
	AddNonTerminalsUnion(newGram, gram2, gram2.terminals);

	AddTerminals(newGram, gram1);
	AddTerminals(newGram, gram2);

	AddProductionRulesUnion(newGram, gram1);
	AddProductionRulesUnion(newGram, gram2);

	return newGram;
}

// Product functions
void AddNonTerminalsProduct3(Grammar& newGram, Grammar& otherGram, std::vector<std::string> terminals) { 
	// adds the non-terminals for grammars of type 3 closed under product
	if (newGram.nonTerminals.empty()) { // if we don't have any other non-terminals
		for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
			bool alreadyExists = false;
			do {
				alreadyExists = false;
				std::string foundString;
				for (std::vector<std::string>::iterator itT = terminals.begin(); itT != terminals.end() && !alreadyExists; itT++) {
					if ((*itT).compare(*itO) == 0) { // if the terminal already exists in the grammar we are trying to make
						alreadyExists = true;
						foundString = *itO;

						(*itO).push_back('\'');
					}
				}

				if (alreadyExists) {
					for (std::vector<std::pair<std::string, std::string>>::iterator itPair = otherGram.productionRules.begin(); itPair != otherGram.productionRules.end(); itPair++) {
						const size_t offsetFirst = (*itPair).first.find(foundString);
						if (offsetFirst != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetFirst; i++) {
								newStr.push_back((*itPair).first[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetFirst + 1; i < (*itPair).first.size(); i++) {
								newStr.push_back((*itPair).first[i]);
							}
							(*itPair).first = newStr;
						}

						const size_t offsetSecond = (*itPair).second.find(foundString);
						if (offsetSecond != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetSecond; i++) {
								newStr.push_back((*itPair).second[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetSecond + 1; i < (*itPair).second.size(); i++) {
								newStr.push_back((*itPair).second[i]);
							}
							(*itPair).second = newStr;
						}
					}

					if (foundString.compare(gram1.startingPoint) == 0) {
						gram1.startingPoint = *itO;
					}
				}
			} while (alreadyExists);
			newGram.nonTerminals.push_back(*itO);
		}

		newGram.startingPoint = gram1.startingPoint;
	}
	else {
		for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
			bool alreadyExists = false;
			do {
				alreadyExists = false;
				std::string foundString;
				for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
					if ((*itN).compare(*itO) == 0) { // if the non-terminal already exists in the grammar we are trying to make
						alreadyExists = true;
						foundString = *itO;

						(*itO).push_back('\'');
					}
				}

				if (!alreadyExists) {
					for (std::vector<std::string>::iterator itT = newGram.terminals.begin(); itT != newGram.terminals.end() && !alreadyExists; itT++) {
						if ((*itT).compare(*itO) == 0) { // if the terminal already exists in the grammar we are trying to make
							alreadyExists = true;
							foundString = *itO;

							(*itO).push_back('\'');
						}
					}

					if (foundString.compare(gram2.startingPoint) == 0) {
						gram2.startingPoint = *itO;
					}
				}

				if (alreadyExists) {
					for (std::vector<std::pair<std::string, std::string>>::iterator itPair = otherGram.productionRules.begin(); itPair != otherGram.productionRules.end(); itPair++) {
						const size_t offsetFirst = (*itPair).first.find(foundString);
						if (offsetFirst != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetFirst; i++) {
								newStr.push_back((*itPair).first[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetFirst + 1; i < (*itPair).first.size(); i++) {
								newStr.push_back((*itPair).first[i]);
							}
							(*itPair).first = newStr;
						}

						const size_t offsetSecond = (*itPair).second.find(foundString);
						if (offsetSecond != std::string::npos) {
							std::string newStr;
							for (size_t i = 0; i <= offsetSecond; i++) {
								newStr.push_back((*itPair).second[i]);
							}
							newStr.push_back('\'');
							for (size_t i = offsetSecond + 1; i < (*itPair).second.size(); i++) {
								newStr.push_back((*itPair).second[i]);
							}
							(*itPair).second = newStr;
						}
					}

					if (foundString.compare(gram2.startingPoint) == 0) {
						gram2.startingPoint = *itO;
					}
				}
			} while (alreadyExists);
			newGram.nonTerminals.push_back(*itO);
		}
	}
}

void AddProductionRulesProduct012(Grammar& newGram, Grammar& gram1, Grammar& gram2) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = gram1.productionRules.begin(); itO != gram1.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = gram2.productionRules.begin(); itO != gram2.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}

	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, (gram1.startingPoint + gram2.startingPoint)));
}

void AddProductionRulesProduct3(Grammar& newGram, Grammar& gram1, Grammar& gram2) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = gram1.productionRules.begin(); itO != gram1.productionRules.end(); itO++) {
		bool found = false;
		for (std::vector<std::string>::iterator itNT = gram1.nonTerminals.begin(); itNT != gram1.nonTerminals.end(); itNT++) {
			if (((*itO).second).find(*itNT) != std::string::npos) {
				found = true;
				break;
			}
		}

		if (!found) {
			(*itO).second.append(gram2.startingPoint);
		}

		newGram.productionRules.push_back(*itO);
	}

	for (std::vector<std::pair<std::string, std::string>>::iterator itO = gram2.productionRules.begin(); itO != gram2.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}
}

Grammar CreateGrammarFromProduct(Grammar gram1, Grammar gram2) {
	Grammar newGram;

	GrammarType gram1Type = FindGrammarType(gram1);
	if (gram1Type == GrammarType::Type3 && gram1Type == FindGrammarType(gram2)) {
		AddNonTerminalsProduct3(newGram, gram1, gram2.terminals);
		AddNonTerminalsProduct3(newGram, gram2, gram2.terminals);

		AddTerminals(newGram, gram1);
		AddTerminals(newGram, gram2);

		AddProductionRulesProduct3(newGram, gram1, gram2);
	}
	else {
		AddNonTerminalsUnion(newGram, gram1, gram2.terminals);
		AddNonTerminalsUnion(newGram, gram2, gram2.terminals);

		AddTerminals(newGram, gram1);
		AddTerminals(newGram, gram2);

		AddProductionRulesProduct012(newGram, gram1, gram2);
	}

	return newGram;
}

// Closure functions
void AddNonTerminalsClosure01(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
		newGram.nonTerminals.push_back(*itO); // we just add all of them without worry
	}

	newGram.startingPoint.push_back('S'); // we make a new starting point

	bool alreadyExists;
	do {
		alreadyExists = false;
		for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
			if ((*itN).compare(newGram.startingPoint) == 0) { // we check if our new starting point already exists in our nonTerminals
				alreadyExists = true;
			}
		}

		if (alreadyExists) { // if it does already exist
			newGram.startingPoint.push_back('\''); // then add another ' to hopefully differentiate it from the others
		}
	} while (alreadyExists);

	newGram.nonTerminals.push_back(newGram.startingPoint); // finally, we add the starting point to our list of non-terminals

	std::string newNT = "X"; // we add a new non-terminal

	do {
		alreadyExists = false;
		for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
			if ((*itN).compare(newNT) == 0) { // we check if our new non-terminal already exists in our nonTerminals
				alreadyExists = true;
			}
		}

		if (alreadyExists) { // if it does already exist
			newNT.push_back('\''); // then add another ' to hopefully differentiate it from the others
		}
	} while (alreadyExists);

	newGram.nonTerminals.push_back(newNT); // finally, we add the non-terminal to our list of non-terminals
}

void AddProductionRulesClosure01(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = otherGram.productionRules.begin(); itO != otherGram.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}

	std::string newNT = newGram.nonTerminals[newGram.nonTerminals.size() - 1];

	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, otherGram.startingPoint));
	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, "|"));
	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, (newNT + otherGram.startingPoint)));

	for (std::vector<std::string>::iterator itO = otherGram.terminals.begin(); itO != otherGram.terminals.end(); itO++) {
		newGram.productionRules.push_back(std::pair<std::string, std::string>((newNT + (*itO)), (otherGram.startingPoint + *itO)));
		newGram.productionRules.push_back(std::pair<std::string, std::string>((newNT + (*itO)), (newNT + otherGram.startingPoint + *itO)));
	}
}

void AddNonTerminalsClosure23(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::string>::iterator itO = otherGram.nonTerminals.begin(); itO != otherGram.nonTerminals.end(); itO++) {
		newGram.nonTerminals.push_back(*itO); // we just add all of them without worry
	}

	newGram.startingPoint.push_back('S'); // we make a new starting point

	bool alreadyExists;
	do {
		alreadyExists = false;
		for (std::vector<std::string>::iterator itN = newGram.nonTerminals.begin(); itN != newGram.nonTerminals.end() && !alreadyExists; itN++) {
			if ((*itN).compare(newGram.startingPoint) == 0) { // we check if our new starting point already exists in our nonTerminals
				alreadyExists = true;
			}
		}

		if (alreadyExists) { // if it does already exist
			newGram.startingPoint.push_back('\''); // then add another ' to hopefully differentiate it from the others
		}
	} while (alreadyExists);

	newGram.nonTerminals.push_back(newGram.startingPoint); // finally, we add the starting point to our list of non-terminals
}

void AddProductionRulesClosure2(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = otherGram.productionRules.begin(); itO != otherGram.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}

	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, (newGram.startingPoint + otherGram.startingPoint)));
	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, "|"));
}

void AddProductionRulesClosure3(Grammar& newGram, Grammar& otherGram) {
	for (std::vector<std::pair<std::string, std::string>>::iterator itO = otherGram.productionRules.begin(); itO != otherGram.productionRules.end(); itO++) {
		newGram.productionRules.push_back(*itO); // we just add all of them without worry
	}

	for (std::vector<std::pair<std::string, std::string>>::iterator itO = otherGram.productionRules.begin(); itO != otherGram.productionRules.end(); itO++) {
		bool found = false;
		for (std::vector<std::string>::iterator itNT = otherGram.nonTerminals.begin(); itNT != otherGram.nonTerminals.end(); itNT++) {
			if (((*itO).second).find(*itNT) != std::string::npos) {
				found = true;
				break;
			}
		}

		if (!found) {
			(*itO).second.append(otherGram.startingPoint);
			newGram.productionRules.push_back(*itO);
		}
	}

	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, otherGram.startingPoint));
	newGram.productionRules.push_back(std::pair<std::string, std::string>(newGram.startingPoint, "|"));
}

Grammar CreateGrammarFromClosure(Grammar gram1) {
	Grammar newGram;
	
	switch (FindGrammarType(gram1)) {
		case GrammarType::Type0:
		case GrammarType::Type1: {
			AddNonTerminalsClosure01(newGram, gram1);

			AddTerminals(newGram, gram1);

			AddProductionRulesClosure01(newGram, gram1);
			break;
		}
		case GrammarType::Type2: {
			AddNonTerminalsClosure23(newGram, gram1);

			AddTerminals(newGram, gram1);

			AddProductionRulesClosure2(newGram, gram1);
			break;
		}
		case GrammarType::Type3: {
			AddNonTerminalsClosure23(newGram, gram1);

			AddTerminals(newGram, gram1);

			AddProductionRulesClosure3(newGram, gram1);
			break;
		}
	}

	return newGram;
}

// Menu/Reading functions

Grammar ReadGrammar() {
	Grammar newGram;
	int num = -1;
	do {
		std::cout << "How many non-terminals do you want?\n"; std::cin >> num;
	} while (num < 0);

	for (int i = 0; i < num; i++) {
		std::string str1;
		std::cout << "\nRead non-terminal number " << i + 1 << ": "; std::cin >> str1;

		newGram.nonTerminals.push_back(str1);
	}

	num = -1;
	do {
		std::cout << "\nHow many terminals do you want?\n"; std::cin >> num;
	} while (num < 0);

	for (int i = 0; i < num; i++) {
		std::string str1;
		std::cout << "\nRead terminal number " << i + 1 << ": "; std::cin >> str1;

		newGram.terminals.push_back(str1);
	}

	if (newGram.nonTerminals.size() > 0) {
		std::cout << "\nRead the number of the non-terminal you want to be the starting point.\n";
		int count = 1;
		for (std::vector<std::string>::iterator itNT = newGram.nonTerminals.begin(); itNT != newGram.nonTerminals.end(); itNT++, count++) {
			std::string nT = *itNT;
			std::cout << count << ". " << nT << '\n';
		}

		int num = -1;
		do {
			std::cin >> num;
		} while (num < 1 && num > newGram.nonTerminals.size());

		newGram.startingPoint = newGram.nonTerminals[num - 1];
	}

	num = -1;
	do {
		std::cout << "\nHow many production rules do you want?\n"; std::cin >> num;
	} while (num < 0);

	for (int i = 0; i < num; i++) {
		std::string str1, str2;
		std::cout << "\n\nRead left-hand side of production rule number " << i + 1 << ": "; std::cin >> str1;
		std::cout << "\nRead right-hand side of production rule number " << i + 1 << ": "; std::cin >> str2;
		newGram.productionRules.push_back(std::pair<std::string, std::string>(str1, str2));
	}

	return newGram;
}

bool RunMenu()
{
	int caseNum = 0; Grammar selectedGram;
	do {
		std::cout << "\n\n\nInput your choice.";
		std::cout << "\n1.Read first grammar.";
		std::cout << "\n2.Read second grammar.";
		std::cout << "\n3.Select and print said grammar.";
		std::cout << "\n4.Select and show the type of said grammar.";
		std::cout << "\n5.Devise a new grammar under the union of the first and second grammars.";
		std::cout << "\n6.Devise a new grammar under the product of the first and second grammars.";
		std::cout << "\n7.Select and devise a new grammar under the Kleene Closure of selected grammar.";
		std::cin >> caseNum;
		system("cls");
	} while (caseNum < 1 || caseNum > 7);

	if (caseNum == 3 || caseNum == 4 || caseNum == 7) {
		int gramNum = -1;
		do {
			std::cout << "\n\n\nInput your choice.";
			std::cout << "\n1.Select first grammar.";
			std::cout << "\n2.Select second grammar.";
			std::cin >> gramNum;
			system("cls");
		} while (gramNum != 1 && gramNum != 2);

		selectedGram = (gramNum == 1) ? gram1 : gram2;
	}

	switch (caseNum) {
		case 1: {
			gram1 = ReadGrammar();
			break;
		}
		case 2: {
			gram2 = ReadGrammar();
			break;
		}
		case 3: {
			PrintGrammar(selectedGram);
			break;
		}
		case 4: {
			switch (FindGrammarType(selectedGram)) {
				case GrammarType::Type0: {
					std::cout << "\nSelected grammar is of type 0.";
					break;
				}
				case GrammarType::Type1: {
					std::cout << "\nSelected grammar is of type 1.";
					break;
				}
				case GrammarType::Type2: {
					std::cout << "\nSelected grammar is of type 2.";
					break;
				}
				case GrammarType::Type3: {
					std::cout << "\nSelected grammar is of type 3.";
					break;
				}
			}
			break;
		}
		case 5: {
			Grammar resultingGrammar = CreateGrammarFromUnion(gram1, gram2);
			PrintGrammar(resultingGrammar);
			break;
		}
		case 6: {
			Grammar resultingGrammar = CreateGrammarFromProduct(gram1, gram2);
			PrintGrammar(resultingGrammar);
			break;
		}
		case 7: {
			Grammar resultingGrammar = CreateGrammarFromClosure(selectedGram);
			PrintGrammar(resultingGrammar);
			break;
		}
	}

	char ans = '\0';
	do {
		std::cout << "\n\nDo you wish to keep running this program?"; std::cin >> ans;
	} while (!strchr("yYnN", ans));

	return (strchr("yY", ans));
}

int main() {
	gram1.nonTerminals.push_back("A");
	gram1.nonTerminals.push_back("B");
	gram1.nonTerminals.push_back("C");
	gram1.nonTerminals.push_back("D");
	gram1.nonTerminals.push_back("E");
	gram1.nonTerminals.push_back("F");
	gram1.nonTerminals.push_back("S");
	gram1.terminals.push_back("a");
	gram1.terminals.push_back("b");
	gram1.terminals.push_back("c");
	gram1.terminals.push_back("d");
	gram1.terminals.push_back("0");
	gram1.terminals.push_back("1");
	gram1.terminals.push_back("5");
	gram1.startingPoint = "S";
	gram1.productionRules.push_back(std::pair<std::string, std::string>("S", "aA"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("S", "bB"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("S", "cC"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("S", "dD"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("A", "1"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("B", "5"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("C", "5E"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("C", "1F"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("D", "10"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("E", "1"));
	gram1.productionRules.push_back(std::pair<std::string, std::string>("F", "5"));

	gram2.nonTerminals.push_back("X");
	gram2.nonTerminals.push_back("Y");
	gram2.nonTerminals.push_back("Z");
	gram2.nonTerminals.push_back("W");
	gram2.nonTerminals.push_back("R");
	gram2.nonTerminals.push_back("S");
	gram2.terminals.push_back("x");
	gram2.terminals.push_back("y");
	gram2.terminals.push_back("w");
	gram2.terminals.push_back("z");
	gram2.terminals.push_back("0");
	gram2.terminals.push_back("1");
	gram2.terminals.push_back("5");
	gram2.startingPoint = "S";
	gram2.productionRules.push_back(std::pair<std::string, std::string>("S", "xX"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("S", "yY"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("S", "zZ"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("S", "wW"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("Y", "1R"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("R", "1"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("X", "1"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("Z", "5"));
	gram2.productionRules.push_back(std::pair<std::string, std::string>("W", "10"));

	while (RunMenu());
	return 0;
}