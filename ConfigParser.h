#if !defined(__CD_CONFIGPARSER_H)
#define __CD_CONFIGPARSER_H

// Configuration File Parser
// Copyright (c) 2004, Jeff Schiller (CodeDread)
//
// Please send review comments to Jeff@CodeDread.com

/// \mainpage Configuration File Parser API Documentation
///
/// \section copyright Copyright Information
///
/// Copyright (C) 2004.  Jeff Schiller (CodeDread)
///
/// \section revhist Revision History
///
/// <TABLE>
/// <TR>
///	<TD ALIGN="CENTER"><B>Version</B></TD>	<TD><B>Date</B></TD> <TD><B>Notes</B></TD></TR>
/// <TR><TD ALIGN="CENTER">0.1</TD>	<TD>2004-04-15</TD> <TD>Initial version, contains support getting and adding properties, reading, parsing and writing configuration information</TD>
/// <TR><TD ALIGN="CENTER">0.2</TD>	<TD>2004-04-16</TD> <TD>Added methods for removing properties and clearing the configuration.  Corrected write/debug methods to include semicolon.  Corrected parser to handle multiple simultaneous comments within an assignment.  Improved documentation.</TD></TR>
/// <TR><TD ALIGN="CENTER">0.3</TD>	<TD>2004-04-23</TD> <TD>Moved existing configuration file parser to the "Simple" namespace with appropriate class name changes.  Added a configuration file parser that remembers comments, whitespace and the order of all statements.</TD></TR>
/// <TR><TD ALIGN="CENTER">0.4</TD>	<TD>2004-04-24</TD> <TD>All AddXXXX functions now return a pointer to the value instead of a bool.  Added a method to get the property table to ConfigManager class.  Added a flag to ConfigManager::read() to indicate whether comment/whitespace formatting should be preserved.  Removed the "Simple" namespace, the same functionality can now be achieved using ConfigManager but indicating that comments/whitespace should not be preserved.  Fully documented the interface.</TD></TR>
/// <TR><TD ALIGN="CENTER">0.41</TD>	<TD>2004-04-25</TD> <TD>Added a means to explicitly set the Preserve Formatting flag in ConfigManager.  Fixed duplicate eol being written to each line of file.</TD></TR>
/// <TR><TD ALIGN="CENTER">0.42</TD>	<TD>2004-04-26</TD> <TD>Fixed a bug:  String variables were including the enclosing double-quote characters, now they are removed.</TD></TR>
/// </TABLE>
///
/// \section license License Stuff
///
/// This library is free software; you can redistribute it and/or
/// modify it under the terms of the GNU Lesser General Public
/// License as published by the Free Software Foundation; either
/// version 2.1 of the License, or (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
/// Lesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public
/// License along with this library; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
///
/// \section intro Introduction
///
/// This library provides a mechanism to parse a configuration file.  An example of 
/// the config file format which this library supports is shown here:
///
/// sample.cfg:
/// \code
/// // C++ style comments supported
/// /* C-style comments 
///   also supported
///   */
///   
/// AutoStartOn = false; // boolean value
///
/// BuildNumber =44; // integer property
///
/// Version=1.2; // real property
/// 
/// AppName= "CodeDread"; // string property
///
/// // a composite property
/// Player {  // another comment 
/// 	Number = // inline comments allowed
///			1;
///			
///	Name = "Jeff";
///	
///	// composite property
///	Backpack {
///		Color = "Brown";
///		Size = 10;
///	}
///}
///
/// \endcode
///
/// The above cfg file looks a lot like C/C++ code and thus makes it convenient for
/// syntax highlighting in Development IDEs.
///
/// The parser can remember whitespace and all text formatting such that when it re-writes
/// the configuration file it can preserve the file format.
///
/// The formal EBNF notation for the configuration file grammar is located on the 
/// \ref grammar page.
///
/// A ConfigManager object has the ability to read in a configuration file, parse it
/// and store the configuration data in memory.  The configuration data is available
/// for access, modification and removal.  The ConfigManager object can also write
/// the configuration data back to a file (with the option of preserving comments and
/// whitespace or removing it.
///
/// Some example code may help.  All of the below examples use the above configuration file:
/// 
/// <a href="main.html#ReadCfg">Reading a Configuration File</a><br>
/// <a href="main.html#GetSimple">Getting Simple Property Values</a><br>
/// <a href="main.html#GetComp">Getting Composite Property Values</a><br>
/// <a href="main.html#ChangeProp">Changing Property Values</a><br>
/// <a href="main.html#AddProp">Adding Properties</a><br>
/// <a href="main.html#RemoveProp">Removing Properties</a><br>
/// <a href="main.html#WriteCfg">Writing a Configuration File</a><br>
///
/// <h2 id="ReadCfg">Reading a Configuration File</h2>
/// 
/// Create a ConfigManager object and invoke the read() method on it.
/// 
/// \code
/// ConfigManager config;
/// 
/// // read in configuration file
/// if(!config.read("sample.cfg")) {
///		cerr << "Error reading file" << endl;
///		exit(-1);
/// }
/// \endcode
///
/// 
/// <h2 id="GetSimple">Getting Simple Property Values</h2>
///
/// To retrieve a property value, call the getXXXXProperty() methods.  If the property is not found, you can use the addXXXXProperty() method.
/// 
/// \code
/// // get AutoSaveOn bool property
/// bool* pbAutoSaveOn = NULL;
/// if( !config.getBoolProperty("AutoSaveOn", &pbAutoSaveOn)) {
///		pbAutoSaveOn = config.addBoolProperty("AutoSaveOn", true);
/// }
/// assert(pbAutoSaveOn);
/// cout << "AutoSaveOn = " << *pbAutoSaveOn << endl;
///
/// // get AppName string property
/// string* psAppName = NULL;
/// if( !config.getStringProperty("AppName", &psAppName)) {
///		psAppName = config.addStringProperty("AppName", "MyApp");
/// }
/// assert(psAppName);
/// cout << "AppName = " << *psAppName << endl;
/// \endcode
///
/// <h2 id="GetComp">Getting Composite Properties</h2>
///
/// To retrieve a composite property, use the getCompositeProperty() method.  To recurse several levels deep, continue to call the getCompositeProperty() method:
/// 
/// \code
/// // get Player.Backpack.Size integer property
/// CompositeAssignment* pcPlayer = NULL;
/// CompositeAssignment* pcBackpack = NULL;
/// int* pcBPSize = NULL;
/// // we can use one if to retrieve Player.Backpack.Size if we want:
/// if( !config.getCompositeProperty("Player", &pcPlayer) || !pcPlayer ||
///		!pcPlayer->getCompositeProperty("Backpack", &pcBackpack) || !pcBackpack ||
///		!pcBackback->getIntProperty("Size", &pcBPSize) || !pcBPSize) 
/// {
///		cerr << "Player.Backpack.Size property not found" << endl;
///		exit(-1);
/// }
/// cout << "Player.Backpack.Size = " << *pcBPSize << endl;
/// \endcode
///
/// <h2 id="ChangeProp">Changing Property Values</h2>
///
/// To change a property value, update the pointed-to values:
/// 
/// \code
/// *pbAutoSaveOn = false; // AutoSaveOn now equals false
/// 
/// *pcBPSize = 20; // Player.Backpack.Size now equals 20
/// \endcode
///
/// <h2 id="AddProp">Adding Properties</h2>
/// 
/// To add a completely new property to the configuration data, invoke the addXXXXProperty method on the appropriate object:
///
/// \code
/// // add another string property:
/// string* psAuthor = config.addStringProperty("AppAuthor", "Jeff Schiller");
/// 
/// // add a bool property to the Player.Backpack composite:
/// bool* pbBackpackFilled = pcBackpack->addBoolProperty("Filled", false);
/// \endcode
///
/// <h2 id="RemoveProp">Removing Properties</h2>
/// 
/// Removal of a property simply requires invocation of the removeProperty method:
/// 
/// \code
/// if(config.removeProperty("Version")) {
///		cout << "Version property was removed" << endl;
/// }
/// \endcode
/// 
/// <h2 id="WriteCfg">Writing a Configuration File</h2>
///
/// Perhaps the most simplest operation of all, simply invoke the write() method on the ConfigManager object:
///
/// \code
/// // Write configuration file back to disk
/// config.write("sample.cfg");
/// \endcode

/// \page grammar
/// The following is the format EBNF structure of the Configuration File format supported
/// by ConfigParser:
/// 
/// \code
/// // commments and whitespace
/// <c-style comment> ::= "/*" <any printable character> "*/"
/// <cpp-style comment> ::= "//" (<any printable character> - <end of line>) <end of line>
/// <comment> ::= (c-style comment) | (cpp-style comment) | <whitespace>
///
/// // simple property values
/// <bool value> ::= "true" | "false"
/// <real value> ::= ['-'] (<digit>)+ '.' (<digit>)*
/// <int value> ::= ['-'] (<digit>)+
/// <escape code> ::= '\' ('a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' | '"' | '\')
/// <string value> ::= '"' ((<any printable character> - '"' - '\') | <escape code>)* '"'
/// <simple value> ::= (<bool value> | <real value> | <int value> | <string value>)
///
/// // property name
/// <property name> ::= ( (<alpha> | '_') (<alpha> | <digit> | '_') ) - ("true" | "false")
///
/// // Assignment
/// <assignment> ::= (<simple assignment> | <composite assignment>)
/// <simple assignment> ::= <property name> <comment>* '=' <comment>* <simple value> <comment>* ';'
/// <composite assignment> ::= <property name> <comment>* '{' <configuration> '}'
/// 
/// // Configuration (base rule)
/// <configuration> ::= (<comment> | <assignment>)*
/// \endcode
///

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(CONFIGPARSER_EXPORTS)
#define CONFIGPARSER_DLL_EXPORTS __declspec(dllexport)
#else
#define CONFIGPARSER_DLL_EXPORTS __declspec(dllimport)
#endif
//#define CONFIGPARSER_DLL_EXPORTS

#include <iostream>
#include <string>
#include <map>
#include <vector>

#pragma warning( disable: 4251 )


namespace ConfigParser {

/// This enumeration provides a means of distinguishing a property's type
enum EPropertyType { EVoid, EString, EBool, EInt, EReal, EComposite };

/// The CfgFileElement class is the abstract base class for all elements to 
/// be parsed by the ConfigManager.  The pure virtual function write() must
/// be implemented by a concrete subclass.
class CONFIGPARSER_DLL_EXPORTS CfgFileElement
{
public:
	/// This method writes the element out to a stream.
	///
	/// \param o An output stream that is ready for text.
	/// \param bPreserveFormatting Indicates whether comments and whitespace
	/// formatting are preserved.
	virtual void write(std::ostream& o, bool bPreserveFormatting) const = 0;
};

/// The Comment class represents comments and whitespace formatting.  In 
/// other words, the Comment class holds all formatting that cannot be 
/// parsed into properties and their value.
class CONFIGPARSER_DLL_EXPORTS Comment : public CfgFileElement
{
	std::string strComment;
public:
	Comment(const std::string& s);
	~Comment();

	virtual void write(std::ostream& o, bool bPreserveFormatting = true) const;
};

/// The Assignment class is the base class for a property and its values.  It is
/// subclassed by the SimpleAssignment and the CompositeAssignment classes.  This 
/// class includes the Property Name and any comments/whitespace that follows the
/// Property Name.
class CONFIGPARSER_DLL_EXPORTS Assignment : public CfgFileElement {
	std::string m_strPropertyName;
	std::vector<Comment*> m_vCommentsPostName;
public:
	Assignment();
	Assignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName);
	virtual ~Assignment();

	void addCommentPostName(Comment* c);
	const std::string& getPropertyName() const;

	virtual void write(std::ostream& o, bool bPreserveFormatting = true) const;

};

/// The SimpleAssignment class is the base class for all simple properties.  It
/// contains all comments/whitespace that follows the equals sign and all 
/// comments/whitespace that follows the property value.  See the following 
/// concrete base classes for implementation:  BoolAssignment, RealAssignment,
/// IntAssignment and StringAssignment.
class CONFIGPARSER_DLL_EXPORTS SimpleAssignment : public Assignment
{
protected:
	std::vector<Comment*> m_vCommentsPostEquals;
	std::vector<Comment*> m_vCommentsPostValue;
public:
	SimpleAssignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName,
		const std::vector<Comment*>& vCommentsPostEquals);
	virtual ~SimpleAssignment();
	
	void addCommentPostEquals(Comment* c);
	void addCommentPostValue(Comment* c);


	virtual void writeValue(std::ostream& o) const = 0;

	virtual void write(std::ostream& o, bool bPreserveFormatting = true) const;
};

/// The BoolAssignment class represents a Boolean Property.
class CONFIGPARSER_DLL_EXPORTS BoolAssignment : public SimpleAssignment
{
	friend class CompositeAssignment;
protected:
	bool m_value;
public:
	BoolAssignment(const std::string name, const std::vector<Comment*>& vCommentsPostName,
		const std::vector<Comment*>& vCommentsPostEquals, bool b);
	virtual ~BoolAssignment();

	bool getValue() const;
	void setValue(bool b);

	virtual void writeValue(std::ostream& o) const;
};

/// The StringAssignment class represents a String Property.
class CONFIGPARSER_DLL_EXPORTS StringAssignment : public SimpleAssignment
{
	friend class CompositeAssignment;
protected:
	std::string m_value;
public:
	StringAssignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName,
		const std::vector<Comment*>& vCommentsPostEquals, const std::string& v);
	virtual ~StringAssignment();

	const std::string& getValue() const;
	void setValue(const std::string& v);

	virtual void writeValue(std::ostream& o) const;
};

/// The IntAssignment class represents an Integer Property.
class CONFIGPARSER_DLL_EXPORTS IntAssignment : public SimpleAssignment
{
	friend class CompositeAssignment;
protected:
	int m_value;
public:
	IntAssignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName,
		const std::vector<Comment*>& vCommentsPostEquals, int v);
	virtual ~IntAssignment();

	int getValue() const;
	void setValue(int v);

	virtual void writeValue(std::ostream& o) const;
};

/// The RealAssignment class represents a Real (floating-point) Property.
class CONFIGPARSER_DLL_EXPORTS RealAssignment : public SimpleAssignment
{
	friend class CompositeAssignment;
protected:
	double m_value;
public:
	RealAssignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName,
		const std::vector<Comment*>& vCommentsPostEquals, double v);
	virtual ~RealAssignment();

	double getValue() const;
	void setValue(double v);

	virtual void writeValue(std::ostream& o) const;
};

/// The CompositeAssignment class represents a property type that can contain 
/// other properties.  Use the Get functions to determine if a property
/// exists and get its value.  Use the Add functions to add a new property
/// or modify an existing property's value.
class CONFIGPARSER_DLL_EXPORTS CompositeAssignment : public Assignment
{
protected:
	// this can be comments, whitespace, simple assignments or composite assignments
	std::vector<CfgFileElement*> m_vElements;

	std::map<std::string, BoolAssignment*> m_mBools;
	std::map<std::string, RealAssignment*> m_mReals;
	std::map<std::string, IntAssignment*> m_mInts;
	std::map<std::string, StringAssignment*> m_mStrings;
	std::map<std::string, CompositeAssignment*> m_mComposites;

	friend class ConfigManager;
	CompositeAssignment() 
	{
	}
public:

	CompositeAssignment(const std::string& name, const std::vector<Comment*>& vCommentsPostName);	
	virtual ~CompositeAssignment();

	void addCfgFileElement(CfgFileElement* ele);

	virtual void write(std::ostream& o, bool bPreserveFormatting = true) const;

	// returns whether a property is contained in this composite (and populates 
	// outPropertyValue if present) 
	bool getStringProperty(const std::string& inPropertyName, std::string** outPropertyValue = NULL); 
	bool getBoolProperty(const std::string& inPropertyName, bool** outPropertyValue = NULL); 
	bool getIntProperty(const std::string& inPropertyName, int** outPropertyValue = NULL); 
	bool getRealProperty(const std::string& inPropertyName, double** outPropertyValue = NULL); 
	bool getCompositeProperty(const std::string& inPropertyName, CompositeAssignment** outPropertyValue = NULL); 

	std::string* addStringProperty(const std::string& inPropertyName, const std::string& inPropertyValue);
	bool* addBoolProperty(const std::string& inPropertyName, bool inPropertyValue);
	int* addIntProperty(const std::string& inPropertyName, int inPropertyValue);
	double* addRealProperty(const std::string& inPropertyName, double inPropertyValue);
	CompositeAssignment* addCompositeProperty(const std::string& inPropertyName, CompositeAssignment* inPropertyValue);	
	
	std::string* addStringProperty(StringAssignment* a);
	bool* addBoolProperty(BoolAssignment* a);
	int* addIntProperty(IntAssignment* a);
	double* addRealProperty(RealAssignment* a);
	CompositeAssignment* addCompositeProperty(CompositeAssignment* a);

	bool removeProperty(const std::string& inPropertyName );

	// returns the number of properties within this composite (and populates 
	// outPropertyNames if present) 
	size_t getPropertyTable(std::vector< std::pair<std::string, EPropertyType> >* outPropertyTable = NULL ) const; 

	void clear();

};

/// The ConfigManager class can read in a configuration file, generate the 
/// configuration information via the read() method and write the configuration 
/// file back via the write() method.  
/// 
/// The ConfigManager is actually derived from the CompositeAssignment class 
/// and represents the "top-level" in the property hierarchy (the top-level 
/// composite property is unnamed).
class CONFIGPARSER_DLL_EXPORTS ConfigManager : public CompositeAssignment {
private:
	bool m_bPreserveFormatting;
public:
	ConfigManager();
	virtual ~ConfigManager();

	/// Gets the current Preserve Formatting mode of the ConfigManager
	bool getPreserveFormatting() const { return m_bPreserveFormatting; }
	/// Explicitly sets the current Preserve Formatting mode of the ConfigManager.
	void setPreserveFormatting(bool b) { m_bPreserveFormatting = b; }

	// use this function to read in the file 
	// optionally capture errors in outErrors 
 	bool read(const char* filename, bool bPreserveFormatting = true, std::vector< std::string >* outErrors = NULL); 

	// This writes the existing configuration into the output file
	bool write(const char* filename);
}; // class ConfigManager


} // namespace ConfigParser

#endif // __CD_CONFIGPARSER_H