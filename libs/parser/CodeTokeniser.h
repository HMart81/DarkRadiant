#ifndef _CODE_TOKENISER_H_
#define _CODE_TOKENISER_H_

#include "iarchive.h"
#include "DefTokeniser.h"
#include <list>

namespace parser
{

// Code tokeniser function, with special treatment for #define statements
class CodeTokeniserFunc
{   
    // Enumeration of states
    enum
	{
        SEARCHING,        // haven't found anything yet
        TOKEN_STARTED,    // found the start of a possible multi-char token
		AFTER_DEFINE,	  // after parsing a #define command
		AFTER_DEFINE_FORWARDSLASH,	// after parsing when encountering a forward slash
        QUOTED,         // inside quoted text, no tokenising
		AFTER_CLOSING_QUOTE, // right after a quoted text, checking for backslash
		SEARCHING_FOR_QUOTE, // searching for continuation of quoted string (after a backslash was found)
        FORWARDSLASH,   // forward slash found, possible comment coming
        COMMENT_EOL,    // double-forwardslash comment
        COMMENT_DELIM,  // inside delimited comment (/*)
        STAR            // asterisk, possibly indicates end of comment (*/)
    } _state;
        
    // List of delimiters to skip
    const char* _delims;
    
    // List of delimiters to keep
    const char* _keptDelims;
    
    // Test if a character is a delimiter
    bool isDelim(char c) {
        const char* curDelim = _delims;
        while (*curDelim != 0) {
            if (*(curDelim++) == c) {
                return true;
            }
        }
        return false;
    }
    
    // Test if a character is a kept delimiter
    bool isKeptDelim(char c) {
        const char* curDelim = _keptDelims;
        while (*curDelim != 0) {
            if (*(curDelim++) == c) {
                return true;
            }
        }
        return false;
    }
    
    
public:

    // Constructor
    CodeTokeniserFunc(const char* delims, const char* keptDelims) : 
		_state(SEARCHING), 
		_delims(delims), 
		_keptDelims(keptDelims)
    {}

    /* REQUIRED. Operator() is called by the boost::tokenizer. This function
     * must search for a token between the two iterators next and end, and if
     * a token is found, set tok to the token, set next to position to start
     * parsing on the next call, and return true.
     */
    template<typename InputIterator, typename Token>
    bool operator() (InputIterator& next, InputIterator end, Token& tok) {

        // Initialise state, no persistence between calls
        _state = SEARCHING;

        // Clear out the token, no guarantee that it is empty
        tok = "";

        while (next != end) {
            
            switch (_state) {

                case SEARCHING:

                    // If we have a delimiter, just advance to the next character
                    if (isDelim(*next)) {
                        ++next;
                        continue;
                    }

                    // If we have a KEPT delimiter, this is the token to return.
                    if (isKeptDelim(*next)) {
                        tok = *(next++);
                        return true;
                    }

                    // Otherwise fall through into TOKEN_STARTED, saving the state for the
                    // next iteration
                    _state = TOKEN_STARTED;

                case TOKEN_STARTED:

                    // Here a delimiter indicates a successful token match
                    if (isDelim(*next) || isKeptDelim(*next))
					{
						// Check the token for possible preprocessor #define
						if (tok == "#define")
						{
							_state = AFTER_DEFINE;
							continue;
						}

                        return true;
                    }

                    // Now next is pointing at a non-delimiter. Switch on this 
                    // character.
                    switch (*next) {
                        
                        // Found a quote, enter QUOTED state, or return the 
                        // current token if we are in the process of building 
                        // one.
                        case '\"':
                            if (tok != "") {
                                return true;
                            }
                            else {
                                _state = QUOTED;
                                ++next;
                                continue; // skip the quote
                            }
            
                        // Found a slash, possibly start of comment
                        case '/':
                            _state = FORWARDSLASH;
                            ++next;
                            continue; // skip slash, will need to add it back if this is not a comment
            
                        // General case. Token lasts until next delimiter.
                        default:
                            tok += *next;
                            ++next;
                            continue;
                    }

				case AFTER_DEFINE:
					// Collect token until EOL is found
					if (*next == '\r' || *next == '\n')
					{
                        _state = SEARCHING;
                        ++next;
                        return true;
                    }
					else if (*next == '/')
					{
						// This could be a (line) comment starting here
						_state = AFTER_DEFINE_FORWARDSLASH;
						++next;
						continue;
					}
                    else {
						tok += *next;
                        ++next;
                        continue; // do nothing
                    }

				case AFTER_DEFINE_FORWARDSLASH:
					if (*next == '/')
					{
						// This is the second forward slash, we're in line comment mode now
						_state = COMMENT_EOL;
						++next;
						continue;
					}
					else if (*next == '*')
					{
						// This is the second forward slash, we're in line comment mode now
						_state = COMMENT_DELIM;
						++next;
						continue;
					}
					else
					{
						// False alarm, add the first slash and this character
						tok += '/';
						tok += *next;

						// Switch back to DEFINE mode
						_state = AFTER_DEFINE;
						continue;
					}
                    
                case QUOTED:
        
                    // In the quoted state, just advance until the closing 
                    // quote. No delimiter splitting is required.
                    if (*next == '\"') {
                        ++next;

						// greebo: We've found a closing quote, but there might be a backslash indicating
						// a multi-line string constant "" \ "", so switch to AFTER_CLOSING_QUOTE mode
						_state = AFTER_CLOSING_QUOTE;
                        continue;
                    }
                    else {
                        tok += *next;
                        ++next;
                        continue;
                    }

				case AFTER_CLOSING_QUOTE:
					// We already have a valid string token in our hands, but it might be continued
					// if one of the next tokens is a backslash

					if (*next == '\\') {
						// We've found a backslash right after a closing quote, this indicates we could
						// proceed with parsing quoted content
						++next;
						_state = SEARCHING_FOR_QUOTE;
						continue;
					}

					// Ignore delimiters
					if (isDelim(*next)) {
                        ++next;
                        continue;
                    }

					// Everything except delimiters and backslashes indicates that
					// the quoted content is not continued, so break the loop.
					// This returns the token and parsing continues.
					// Return TRUE in any case, even if the parsed token is empty ("").
					return true;

				case SEARCHING_FOR_QUOTE:
					// We have found a backslash after a closing quote, search for an opening quote
					
					// Step over delimiters
					if (isDelim(*next)) {
                        ++next;
                        continue;
                    }

					if (*next == '\"') {
						// Found the desired opening quote, switch to QUOTED
						++next;
						_state = QUOTED;
						continue;
					}

					// Everything except delimiters or opening quotes indicates an error
					throw ParseException("Could not find opening double quote after backslash.");
                        
                case FORWARDSLASH:
                
                    // If we have a forward slash we may be entering a comment. The forward slash
                    // will NOT YET have been added to the token, so we must add it manually if
                    // this proves not to be a comment.
                    
                    switch (*next) {
                        
                        case '*':
                            _state = COMMENT_DELIM;
                            ++next;
                            continue;
                            
                        case '/':
                            _state = COMMENT_EOL;
                            ++next;
                            continue;
                            
                        default: // false alarm, add the slash and carry on
                            _state = SEARCHING;
                            tok += "/";
                            // Do not increment next here
                            continue;
                    }
                    
                case COMMENT_DELIM:
                
                    // Inside a delimited comment, we add nothing to the token but check for
                    // the "*/" sequence.
                    
                    if (*next == '*') {
                        _state = STAR;
                        ++next;
                        continue;
                    }
                    else {
                        ++next;
                        continue; // ignore and carry on
                    }

                case COMMENT_EOL:
                
                    // This comment lasts until the end of the line.
                    
                    if (*next == '\r' || *next == '\n') {
                        _state = SEARCHING;
                        ++next;

						// If we have a token after a line comment, return it
						if (tok != "")
						{
							return true;
						}
						else
						{
							continue;
						}
                    }
                    else {
                        ++next;
                        continue; // do nothing
                    }
                    
                case STAR:
                
                    // The star may indicate the end of a delimited comment. 
                    // This state will only be entered if we are inside a 
                    // delimited comment.
                    
                    if (*next == '/') {
                    	// End of comment
                        _state = SEARCHING;
                        ++next;
                        continue;
                    }
                    else if (*next == '*') {
                    	// Another star, remain in the STAR state in case we
                    	// have a "**/" end of comment.
                    	_state = STAR;
                    	++next;
                    	continue;
                    }
                    else {
                    	// No end of comment
                    	_state = COMMENT_DELIM;
                    	++next;
                        continue; 
                    }

            } // end of state switch
        } // end of for loop
        
        // Return true if we have added anything to the token
        if (tok != "")
            return true;
        else
            return false;
    }
    
    // REQUIRED. Reset function to clear internal state
    void reset() {
        _state = SEARCHING;
    }  
};

class SingleCodeFileTokeniser : 
	public DefTokeniser
{
    // Istream iterator type
    typedef std::istream_iterator<char> CharStreamIterator;
    
    // Internal Boost tokenizer and its iterator
    typedef boost::tokenizer<CodeTokeniserFunc,
                             CharStreamIterator,
                             std::string> CharTokeniser;
    CharTokeniser _tok;
    CharTokeniser::iterator _tokIter;

private:
	
	// Helper function to set noskipws on the input stream.
	static std::istream& setNoskipws(std::istream& is) {
		is >> std::noskipws;
		return is;
	}
    
public:

    /** 
     * Construct a SingleCodeFileTokeniser with the given input stream, and optionally
     * a list of separators.
     * 
     * @param str
     * The std::istream to tokenise. This is a non-const parameter, since tokens
     * will be extracted from the stream.
     * 
     * @param delims
     * The list of characters to use as delimiters.
     * 
     * @param keptDelims
     * String of characters to treat as delimiters but return as tokens in their
     * own right.
     */
    SingleCodeFileTokeniser(std::istream& str, 
                      const char* delims = WHITESPACE, 
                      const char* keptDelims = "{}(),")
    : _tok(CharStreamIterator(setNoskipws(str)), // start iterator
           CharStreamIterator(), // end (null) iterator
           CodeTokeniserFunc(delims, keptDelims)),
      _tokIter(_tok.begin())
    { }
        
    /** 
     * Test if this StringTokeniser has more tokens to return.
     * 
     * @returns
     * true if there are further tokens, false otherwise
     */
    bool hasMoreTokens() {
        return _tokIter != _tok.end();
    }

    /** 
     * Return the next token in the sequence. This function consumes
     * the returned token and advances the internal state to the following
     * token.
     * 
     * @returns
     * std::string containing the next token in the sequence.
     * 
     * @pre
     * hasMoreTokens() must be true, otherwise an exception will be thrown.
     */
    std::string nextToken() {
        if (hasMoreTokens())
            return *(_tokIter++);
        else
            throw ParseException("DefTokeniser: no more tokens");
    }
    
};

/**
 * High-level tokeniser taking a specific VFS file as input.
 * It is able to handle preprocessor statements like #include 
 * by maintaining several child tokenisers. This can be used
 * to parse code-like files as Doom 3 Scripts or GUIs.
 *
 * Note: Don't expect this tokeniser to be particularly fast.
 */
class CodeTokeniser : 
	public DefTokeniser
{
private:

	struct ParseNode
	{
		ArchiveTextFilePtr archive;
		std::istream inputStream;
		SingleCodeFileTokeniser tokeniser;

		ParseNode(const ArchiveTextFilePtr& archive_,
				const char* delims, const char* keptDelims) :
			archive(archive_),
			inputStream(&archive->getInputStream()),
			tokeniser(inputStream, delims, keptDelims)
		{}
	};
	typedef boost::shared_ptr<ParseNode> ParseNodePtr;
	
	// The stack of child tokenisers
	typedef std::list<ParseNodePtr> NodeList;
	NodeList _nodes;

	NodeList::iterator _curNode;

	// The next token which is not a pre-processor token
	std::string _nextToken;

	// A set of visited files to catch infinite include loops
	typedef std::set<std::string> FileSet;
	FileSet _visitedFiles;

	typedef std::map<std::string, std::string> DefinitionMap;
	DefinitionMap _definitions;

	const char* _delims;
	const char* _keptDelims;

public:

    /** 
     * Construct a CodeTokeniser with the given text file from the VFS.
     */
	CodeTokeniser(const ArchiveTextFilePtr& file,
				  const char* delims = " \t\n\v\r", 
				  const char* keptDelims = "{}(),") :
		_delims(delims),
		_keptDelims(keptDelims)
    {
		_nodes.push_back(ParseNodePtr(new ParseNode(file, _delims, _keptDelims)));
		_curNode = _nodes.begin();

		_visitedFiles.insert(file->getName());

		loadNextRealToken();
	}
        
    bool hasMoreTokens()
	{
		return !_nextToken.empty();
    }

    std::string nextToken()
	{
		std::string temp = _nextToken;

		loadNextRealToken();

		return temp;
    }
    
private:
	void loadNextRealToken()
	{
		while (_curNode != _nodes.end())
		{
			if (!(*_curNode)->tokeniser.hasMoreTokens())
			{
				++_curNode;
			}

			if (_curNode == _nodes.end())
			{
				_nextToken.clear();
			}

			_nextToken = (*_curNode)->tokeniser.nextToken();

			if (!_nextToken.empty() && _nextToken[0] == '#')
			{
				// A pre-processor token is ahead
				handlePreprocessorToken();
				continue;
			}

			// Found a non-preprocessor token

			// Check if this is matching a preprocessor definition
			DefinitionMap::const_iterator found = _definitions.find(_nextToken);
			
			if (found != _definitions.end())
			{
				_nextToken = found->second;
			}

			break;
		}
	}

	void handlePreprocessorToken()
	{
		if (_nextToken == "#include")
		{
			std::string includeFile = (*_curNode)->tokeniser.nextToken();

			ArchiveTextFilePtr file = GlobalFileSystem().openTextFile(includeFile);

			if (file != NULL)
			{
				// Catch infinite recursions
				std::pair<FileSet::iterator, bool> result = _visitedFiles.insert(file->getName());

				if (result.second)
				{
					// Push a new parse node and switch
					_curNode = _nodes.insert(
						_curNode, 
						ParseNodePtr(new ParseNode(file, _delims, _keptDelims))
					);
				}
				else
				{
					globalErrorStream() << "Caught infinite loop on parsing #include token: " 
						<< includeFile << " in " << (*_curNode)->archive->getName() << std::endl;
				}
			}
			else
			{
				globalWarningStream() << "Couldn't find include file: " 
					<< includeFile << " in " << (*_curNode)->archive->getName() << std::endl;
			}
		}
		else if (_nextToken == "#define")
		{
			std::string key = (*_curNode)->tokeniser.nextToken();
			std::string value = (*_curNode)->tokeniser.nextToken();

			std::pair<DefinitionMap::iterator, bool> result = _definitions.insert(
				DefinitionMap::value_type(key, value)
			);

			if (!result.second)
			{
				globalWarningStream() << "Redefinition of " << key 
					<< " in " << (*_curNode)->archive->getName() << std::endl;
			}
		}
		else if (_nextToken == "#undef")
		{
			std::string key = (*_curNode)->tokeniser.nextToken();
			_definitions.erase(key);
		}
		else if (_nextToken == "#ifdef")
		{
			std::string key = (*_curNode)->tokeniser.nextToken();
			DefinitionMap::const_iterator found = _definitions.find(key);

			if (found == _definitions.end())
			{
				skipUntilMatchingEndif();
			}
		}
		else if (_nextToken == "#ifndef")
		{
			DefinitionMap::const_iterator found = _definitions.find(
				(*_curNode)->tokeniser.nextToken());

			if (found != _definitions.end())
			{
				skipUntilMatchingEndif();
			}
		}
		else if (_nextToken == "#if")
		{
			(*_curNode)->tokeniser.skipTokens(1);
		}
	}

	void skipUntilMatchingEndif()
	{
		// Not defined, skip everything until matching #endif
		for (std::size_t level = 1; level > 0;)
		{
			if (!(*_curNode)->tokeniser.hasMoreTokens())
			{
				globalWarningStream() << "No matching #endif for #ifdef in "
					<< (*_curNode)->archive->getName() << std::endl;
			}

			std::string token = (*_curNode)->tokeniser.nextToken();

			if (token == "#endif")
			{
				level--;
			}
			else if (token == "#ifdef" || token == "#ifndef" || token == "#if")
			{
				level++;
			}
		}
	}
};

} // namespace parser

#endif /* _CODE_TOKENISER_H_ */
