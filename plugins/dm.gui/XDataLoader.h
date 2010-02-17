#include "XData.h"
#include "ifilesystem.h"
#include "iarchive.h"
#include "parser/DefTokeniser.h"
#include <map>
#include "generic/callback.h"

#include "reportError.h"


#include "debugging/ScopedDebugTimer.h"


namespace readable
{
	namespace
	{
		/* All vectors of XData-objects are initialized with this size so that no sorting is necessary, which
		would otherwise be necessary when e.g. page2_body was defined before page1_body and a simple
		vector.push_back(..) was used to store the data instead of a direct access using an Index. */
		const int	MAX_PAGE_COUNT			= 20;

		const char*	DEFAULT_TWOSIDED_LAYOUT = "guis/readables/books/book_calig_mac_humaine.gui";
		const char*	DEFAULT_ONESIDED_LAYOUT = "guis/readables/sheets/sheet_paper_hand_nancy.gui";
		const char*	DEFAULT_SNDPAGETURN		= "readable_page_turn";

		const char* XDATA_DIR				= "xdata/";
		const char* XDATA_EXT				= "xd";
	}

	typedef std::map<std::string, std::string> StringMap;
	typedef std::vector<XDataPtr> XDataPtrList;
	struct XDataParse
	{
		XDataPtr xData;
		StringList error_msg;
	};

	///////////////////////////// XDataLoader
	// Class for importing XData from files.
	class XDataLoader
	{
	/* ToDo:
		1) Maybe add a public method for refreshing DefMap.
		2) When a definition couldn't be found refresh the DefMap and try again.
		3) Put the Functor operator and all that stuff into a new class and add a const method pointer, as well as a folder (by Constructor).
		4) Test importing of slash-separated lines. 
		5) Hide operater() in private using friend statements. 
		6) replace XDataParse by pair. 
		7) StoreContent(), ReadLine(), ReadMultiLine() Methods. */

		

	public:
		/* Imports a list of XData objects from the File specified by Filename (just the name, not the path).
		Throws runtime_error exceptions on filesystem-errors, syntax errors and general exceptions.*/
		XDataPtrList import(const std::string& FileName);

		// Required type for the Callback to work.
		typedef const std::string& first_argument_type;

		// Functor operator: Adds all definitions found in the target file to the DefMap.
		void operator() (const std::string& filename);

		void refreshDefMap();

	private:
		StringMap DefMap;

		/**/
		void StoreContent(const std::string& Where, parser::DefTokeniser& tok);

		/* Parses a single definition from a stream into an XData object an generates warning and error messages. */
		XDataParse parseXDataDef(parser::DefTokeniser& tok);

		/* Parses the content between cambered brackets of page-statements. */
		std::string parseText(parser::DefTokeniser& tok);

		/* Handles an import-directive. */
		void importDirective(parser::DefTokeniser& tok, XDataParse& NewXData, const std::string name);

		/* Generates a map that stores all definitions found in all .xd-files and the corresponding .xd-file. */
		void grabAllDefinitions();	//not yet implemented...

		/* Used to jump out of a definition. Can lead to undefined behavior on Syntax-errors. */
		void jumpOutOfBrackets(parser::DefTokeniser& tok, int CurrentDepth);

	//Helper-variables for import.

	};
}