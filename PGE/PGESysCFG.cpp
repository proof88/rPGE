/*
    ###################################################################################
    PGESysCFG.cpp
    This file is part of PGE.
    PR00F's Game Engine cfg file handler
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PRREbaseIncludes.h"  // PCH
#include "PGESysCFG.h"
#include "PGEincludes.h"
#include "PGEpragmas.h"

using namespace std;


// ############################### PUBLIC ################################


PGESysCFG::PGESysCFG(const char* gameTitle) :
    PGEcfgFile(false, false)
{
    getConsole().OLnOI("PGESysCFG::PGESysCFG(%s)", gameTitle);
    
    MAX_PATH;
    nActiveProfile = -1;
    nProfilesCount = 0;
    sFoundProfiles = PGENULL;
    sFoundProfilePlayerNames = PGENULL;

    if ( !findMyDocsFolder() )
    {
        getConsole().EOLnOO("ERROR: couldn't find Documents folder!");
        return;
    }

    ifstream f(PGE_1ST_CFG_FILENAME);
    if ( f.fail() )
    {
        f.close();
        sLangFileName = std::string(PGE_LANG_FOLDER) + "english.txt";
        getConsole().EOLnOO("ERROR: couldn't find %s!", PGE_1ST_CFG_FILENAME);
        return;
    } // f.fail()

    f >> sLangFileName;
    sLangFileName = PGE_LANG_FOLDER + sLangFileName;
    string ggg;
    f >> ggg;
    if ( ggg == "there" )
    {
        bMainCFGinMyDocs = true;
        sPathToProfiles  = sPathToMyDocs;
        sPathToProfiles += gameTitle;
        sPathToProfiles += "\\profiles\\";
    }
    else
    {    // ggg == "here"
        bMainCFGinMyDocs = false;
        char pOrigCurrDirr[1024];
        GetCurrentDirectory(1024, pOrigCurrDirr);
        SetCurrentDirectory( PGE_PROFILE_FOLDER );
        char pNewCurrDirr[1024];
        GetCurrentDirectory(1024, pNewCurrDirr);
        sPathToProfiles = pNewCurrDirr;
        sPathToProfiles += "\\";
        SetCurrentDirectory( pOrigCurrDirr );
    }
    getConsole().OLn("user profile dir is: %s", sPathToProfiles.c_str());

    LoadProfilesList();

    getConsole().SOLnOO("> done!");
    
} // PGESysCFG(...)


PGESysCFG::~PGESysCFG()
{
    getConsole().OLn("PGESysCFG::~PGESysCFG()");
    sPathToMyDocs.clear();
    for (int i = 0; i < nProfilesCount; i++)
    {
        delete sFoundProfiles[i];
        delete sFoundProfilePlayerNames[i];
    }
    delete[] sFoundProfiles;
    delete[] sFoundProfilePlayerNames;
} // ~PGESysCFG()


/**
    Returns access to console preset with logger module name as this class.
    Intentionally not virtual, so the getConsole() in derived class should hide this instead of overriding.

    @return Console instance used by this class.
*/
CConsole& PGESysCFG::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
} // getConsole()


/**
    Returns the logger module name of this class.
    Intentionally not virtual, so derived class should hide this instead of overriding.
    Not even private, so user can also access this from outside, for any reason like controlling log filtering per logger module name.

    @return The logger module name of this class.
*/
const char* PGESysCFG::getLoggerModuleName()
{
    return "PGESysCFG";
} // getLoggerModuleName()


/**
    Returns the path to user documents.
    @return The absolute path to user documents directory. There is always a delimiter character at the end.
*/
string PGESysCFG::getMyDocsFolder()
{
    return sPathToMyDocs;
} // getMyDocsFolder()


/**
    Returns the file name of the selected language file.
*/
string PGESysCFG::getLangFileName() const
{
    return sLangFileName;
} // getLangFileName()


/**
    Reads the language file into the given table.
    @return The read language lines.
*/
int PGESysCFG::readLanguageData(string** &langTable) const
{
    getConsole().OLnOI("PGESysCFG::readLanguageData(...)");
    ifstream f( sLangFileName.c_str() );
    if ( f.fail() )
    {
        getConsole().EOLnOO("ERROR: couldn't open lang file: %s!", sLangFileName.c_str());
        return 0;
    }

    const int nBuffSize = 512;
    int       n         = 0;
    char      pTmp[nBuffSize];
    while ( !f.eof() )
    {
        f.getline(pTmp, nBuffSize);
        if ( (strstr(pTmp, "#") == NULL) && (strlen(pTmp) > 0) )
            n++;
    }
    f.close();

    // seekg() didn't work, the loop below was jumped, neither f.close() now f.open() was working :/
    ifstream g( sLangFileName.c_str() );
    if ( g.fail() )
    {
        getConsole().EOLnOO("ERROR: couldn't open lang file (2nd time): %s!", sLangFileName.c_str());
        return 0;
    }

    langTable = NULL;
    try
    {
        langTable = new string*[n](); // () makes all ptrs NULL
        int i = 0;
        while ( !g.eof() )
        {
            g.getline(pTmp, nBuffSize);
            if ( (strstr(pTmp, "#") == NULL) && (strlen(pTmp) > 0))
            {        
                langTable[i] = new string(pTmp);
                i++;
            }
        }
    }
    catch (const std::bad_alloc&)
    {
        getConsole().EOLnOO("ERROR: memory allocation failure during lang file read!");
        if ( langTable != NULL )
        {
            for (int i = 0; i < n; i++)
            {
                delete langTable[i];
            }
        }
        delete[] langTable;
        return 0;
    }
                                    

    getConsole().SOLnOO("> done!");

    return n;
} // readLanguageData()


/**
    Returns whether player profiles are stored in user documents folder.
    Example: if true, MyDocs\<GAMENAME>\profiles, otherwise program folder\gamedata\profiles.
*/
bool PGESysCFG::areProfilesInMyDocs() const
{
    return bMainCFGinMyDocs;
} // areProfilesInMyDocs()


/**
    Returns the path to the profiles.
    @return The absolute path to game profiles directory. There is always a delimiter character at the end.
*/
string PGESysCFG::getPathToProfiles() const
{
    return sPathToProfiles;
} // getPathToProfiles()


/**
    Returns the number of found profiles.
*/
int PGESysCFG::getProfilesCount() const
{
    return nProfilesCount;
} // getProfilesCount()


/**
    Returns the found profile names.
*/
const string** PGESysCFG::getProfilesList() const
{
    return const_cast<const string**>(sFoundProfiles);
} // getProfilesList()


/**
    Returns the found player names in profiles.
*/
const string** PGESysCFG::getProfilePlayersList() const
{
    return const_cast<const string**>(sFoundProfilePlayerNames);
} // getProfilePlayersList()


/**
    Creates a new profile.
    This creates a profile directory and a profile file in it immediately.
    Creating a new profile won't change the current profile. You have to manually switch to it instead.
    The current profile index may change.

    @param sUser The username for the profile, which will be used primarily for the filename.
                 Thus some characters are restricted (?, *, etc ...).
           sNick The nickname for the profile, which can hold any character.

    @return Non-negative index of the new profile on success.
            -1 if a profile already exists with the given user name.
            -2 if a profile can't be created for some other reason.
*/
int PGESysCFG::addProfile(const char* sUser, const char* sNick)
{
    if ( (sUser == NULL) || (sNick == NULL) )
        return -2;

    getConsole().OLnOI("PGESysCFG::addProfile(%s, %s)", sUser, sNick);

    const std::string sFileToCreate = getPathToProfiles() + sUser + "\\" + sUser + ".cfg";
    const std::string sDirToCreate = getPathToProfiles() + sUser;

    const std::string** const sProfileList = getProfilesList();
    bool bSameUserFound = false;
    for (int i = 0; i < getProfilesCount(); i++)
    {
        bSameUserFound = ( *(sProfileList[i]) == sUser );
        if ( bSameUserFound )
            break;
    }

    if ( bSameUserFound )
    {
        getConsole().EOLnOO("ERROR: same user found!");
        return -1;
    }

    if ( _mkdir( sDirToCreate.c_str() ) != 0 )
    {
        getConsole().EOLnOO("ERROR: couldn't create dir: %s", sDirToCreate.c_str());
        return -2;
    }

    ofstream f_cfg( sFileToCreate );
    if ( f_cfg.fail() )
    {
        getConsole().EOLnOO("ERROR: couldn't create file: %s", sFileToCreate.c_str());
        return -2;
    }

    WriteConfiguration(f_cfg, sUser, sNick);
    f_cfg.close();

    LoadProfilesList();

    const std::string** const sNewProfileList = getProfilesList();
    for (int i = 0; i < getProfilesCount(); i++)
    {
        if ( *(sNewProfileList[i]) == sUser )
        {
            getConsole().SOLnOO("> done!");
            return i;
        }
    }

    // if we have reached this point, we couldn't find the added user, so return error
    getConsole().EOLnOO("ERROR: couldn't find the added profile!");
    return -2;
} // addProfile()


/**
    Deletes the specified profile.
    This deletes the profile directory and the profile file in it immediately.
    If the given index is the current profile, the current profile index will be -1 after deletion.
    Otherwise, the current profile and any loaded cvars won't be changed.
    The current profile index may change.

    @param nIndex The index of the profile to be deleted. Only non-negative values are accepted.

    @return  0 on success.
            -1 if invalid index was given.
            -2 if the profile exists but can't be deleted for some reason.
            -3 profile file deleted but its directory could not be deleted.
*/
int PGESysCFG::deleteProfile(int nIndex)
{
    getConsole().OLnOI("PGESysCFG::deleteProfile(%d)", nIndex);
    if ( (nIndex < 0) || (nIndex >= getProfilesCount()) )
    {
        getConsole().EOLnOO("ERROR: invalid index!");
        return -1;
    }
    if ( nIndex == getProfile() )
    {
        getConsole().OLn("deleting current profile ...");
        SetProfile( -1 );
    }
    const string sUserToDelete = *getProfilesList()[nIndex];
    const string sFileToDelete = getPathToProfiles() + sUserToDelete + "\\" + sUserToDelete + ".cfg";
    const string sDirToDelete = getPathToProfiles() + sUserToDelete;
    
    if ( remove( sFileToDelete.c_str() ) != 0 )
    {
        getConsole().EOLnOO("ERROR: couldn't delete file: %s", sFileToDelete.c_str());
        return -2;
    }

    int retVal = 0;

    if ( _rmdir( sDirToDelete.c_str() ) != 0 )
    {
        // at this point the cfg is deleted, so not a big problem if the directory can't be deleted.
        // if the user stores other files in that dir, those won't be deleted thus the dir can't be deleted.
        // _rmdir() only deletes empty folders, so the functionality is okay.
        getConsole().EOLnOO("WARNING: couldn't delete dir: %s", sDirToDelete.c_str());
        retVal = -3;
    }

    LoadProfilesList();

    getConsole().SOLnOO("> done!");

    return retVal;
} // deleteProfile()


/**
    Gets index of active profile.
*/
int PGESysCFG::getProfile() const
{
    return nActiveProfile;
} // getActiveProfile()


/**
    Sets index of active profile.
    This also implies reading the configuration data for the given profile.
    The active profile index will be -1 if the given index is valid but an error occured while reading config data.
*/
void PGESysCFG::SetProfile(int nIndex)
{
    getConsole().OLnOI("PGESysCFG::SetProfile(%d)", nIndex);
    if ( nIndex >= getProfilesCount() )
    {
        getConsole().EOLnOO("ERROR: invalid index!");
        return;
    }
    if ( nIndex == getProfile() )
    {
        getConsole().OLnOO("current profile index was given, return.");
        return;
    }
    if ( nIndex < -1 )
    {
        getConsole().EOLnOO("ERROR: invalid index!");
        return;
    }

    nActiveProfile = nIndex;
    if ( nIndex == -1 )
    {
        ClearVars();
        getConsole().OO();
        return;
    }

    // at this point, a valid, actual profile index was given which is not the currently set profile
    if ( !readConfiguration() )
    {
        getConsole().EOLnOO("ERROR: readConfiguration() failed!");
        nActiveProfile = -1;
    }
    getConsole().SOLnOO("> done!");
} // SetActiveProfile()


/**
    Gets a cvar by the given name.
    If the given name doesn't exist, the function also creates it.
    Works only if an active profile is selected.
    If you change the value of the cvar, it will be changed only in memory.
    To save changes made to the current profile, use writeConfiguration().

    @return The cvar specified by the given name or a default empty cvar if no profile is selected.
*/
PGEcfgVariable& PGESysCFG::getVar(const char* varName)
{
    if ( getProfile() > -1 )
    {
        PGEcfgVariable& cvar = m_vars[varName];
        return cvar;
    }
    else
        return cvarWrong;
} // getVar()


/**
    Gets a cvar by the given name.
    If the given name doesn't exist, the function won't create it.
    @return The cvar specified by the given name or a default empty cvar if doesn't exist.
*/
const PGEcfgVariable& PGESysCFG::getVar(const char* varName) const
{
    map<string, PGEcfgVariable>::const_iterator it = m_vars.find(varName);
    if ( (m_vars.end() == it) || (getProfile() == -1) )
        return cvarWrong;
    else
        return it->second;
} // getVar()


/**
    Deletes a cvar by the given name.
    You cannot delete the cvar holding the name of the player because it would invalidate the profile.
    If you delete a cvar from the current profile, it will be deleted only on memory-level.
    To save changes made to the current profile, use writeConfiguration().
*/
void PGESysCFG::DeleteVar(const char* varName)
{
    if ( (varName == NULL) || (getProfile() == -1) )
        return;

    if ( strcmp(PGE_SYS_CFG_PLAYER_NAME_CVAR, varName) == 0 )
        return;

    m_vars.erase(varName);
} // DeleteVar()


/**
    Gets the number of cvars.
    @return The number of cvars for the active profile. Always 0 if no profile is selected.
*/
int PGESysCFG::getVarsCount() const
{
    return m_vars.size();
} // getVarsCount()


/**
    Loads configuration for current profile from file.
    @return True on success, false on failure.
*/
bool PGESysCFG::readConfiguration()
{
    getConsole().OLnOI("PGESysCFG::readConfiguration()");
    if ( getProfile() == -1 )
    {
        getConsole().EOLnOO("ERROR: current profile index is -1!");
        return false;
    }

    ClearVars();
    const string sFile = getPathToProfiles() + *sFoundProfiles[getProfile()] + '\\' + (*sFoundProfiles[getProfile()] + ".cfg");
    if ( !load(sFile.c_str()) )
    {
        getConsole().EOLnOO("ERROR: failed to load file: %s! ", sFile.c_str());
        return false;
    }

    getConsole().SOLnOO("> done!");
    return true;
} // readConfiguration()


/**
    Saves configuration for current profile to file.
    @return True on success, false on failure.
*/
bool PGESysCFG::writeConfiguration()
{
    getConsole().OLnOI("PGESysCFG::writeConfiguration()");
    if ( getProfile() == -1 )
    {
        getConsole().EOLnOO("ERROR: current profile index is -1!");
        return false;
    }

    const string sFile = getPathToProfiles() + *sFoundProfiles[getProfile()] + '\\' + (*sFoundProfiles[getProfile()] + ".cfg");
    ofstream f_cfg( sFile );
    if ( f_cfg.fail() )
    {
        getConsole().EOLnOO("ERROR: couldn't open file: %s", sFile.c_str());
        return false;
    }

    WriteConfiguration(f_cfg, *sFoundProfiles[getProfile()], "");
    f_cfg.close();

    getConsole().SOLnOO("> done!");

    return true;
} // writeConfiguration()


// ############################## PROTECTED ##############################


bool PGESysCFG::validateOnLoad(std::ifstream& f) const
{
    string tmp;
    f >> tmp;
    return tmp == PGE_SYS_CFG_FILE_MAGIC_START;
}


// ############################### PRIVATE ###############################


std::string PGESysCFG::sPathToMyDocs;


PGESysCFG::PGESysCFG() :
    PGEcfgFile(false, false)
{

}


PGESysCFG::PGESysCFG(const PGESysCFG& other) :
    PGEcfgFile(other.getAllAcceptedVarsDefineRequirement(), other.getCaseSensitiveVars())
{

} 


PGESysCFG& PGESysCFG::operator=(const PGESysCFG&)
{
    return *this;
}


/**
    Stores user docs folder path in sPathToMyDocs.
    @return True if successful, false on failure.
*/
bool PGESysCFG::findMyDocsFolder()
{
    if ( !sPathToMyDocs.empty() )
    {
        CConsole::getConsoleInstance("PGESysCFG").OLn("findMyDocsFolder(): already set folder: %s", sPathToMyDocs.c_str());
        return true;
    }

    char pPath[MAX_PATH];
    LPITEMIDLIST pIDL = PGENULL;

    if ( SHGetSpecialFolderLocation((HWND)0, CSIDL_PERSONAL, &pIDL) != 0 )
    {
        return false;
    }

    if ( !SHGetPathFromIDList(pIDL, pPath) )
    {   // ms layer for unicode is needed for this feature on win95-98-me (http://msdn.microsoft.com/en-us/library/ms812865.aspx) !!!
        return false;    
    }
    
    sPathToMyDocs.append(pPath).append("\\");
    return true;
} // findMyDocsFolder()


/**
    Gets player name from given cfg file.
*/
bool PGESysCFG::getPlayerNameFromFile(const char* cFilename, std::string& sPlayerName)
{
    ifstream f_cfg(cFilename);

    if ( f_cfg.fail() )
    {
        return false;
    }

    if ( !validateOnLoad(f_cfg) )
    {
        return false;
    }

    const std::streamsize nBuffSize = 1024;
    char cLine[nBuffSize];
    bool l = false;
    while ( (!f_cfg.eof()) && (!l) )
    {
        f_cfg.getline(cLine, nBuffSize);
        l = ( strstr(cLine, PGE_SYS_CFG_PLAYER_NAME_CVAR) != NULL );
    }
    f_cfg.close();
    if ( !l )
        return false;

    // found line containing PGE_SYS_CFG_PLAYER_NAME_CVAR
    PFL::strClr(cLine);
    const std::string sTrimmedLine = cLine;
    std::string sVar;
    bool bParseError = false;

    return lineIsValueAssignment(sTrimmedLine, false, sVar, sPlayerName, bParseError);
} // getPlayerNameFromFile()


/**
    Fills up sFoundProfiles and sFoundProfilePlayerNames.
    Counts the found profiles (directories with cfg file inside them with the same name), stores it in nProfilesCount.
    If the current profile index is valid, it may change during this function but the current profile and loaded cvars
    remain unchanged.
*/
void PGESysCFG::LoadProfilesList()
{
    getConsole().OLnOI("PGESysCFG::LoadProfilesList() ...");
    // save current user
    string sOriginalCurrentProfileUser;
    if ( getProfile() != -1 )
        sOriginalCurrentProfileUser = *( getProfilesList()[getProfile()] );

    nProfilesCount = 0;
    char pOrigCurrDirr[1024];
    char pCurrDirr[1024];
    GetCurrentDirectory(1024, pOrigCurrDirr);
    GetCurrentDirectory(1024, pCurrDirr);
    if ( !SetCurrentDirectory(sPathToProfiles.c_str()) && (GetLastError() != ERROR_SUCCESS) )
    {
        getConsole().EOLnOO("ERROR: couldn't set current dir to: %s", sPathToProfiles.c_str());
        return;
    }

    SetLastError(ERROR_SUCCESS);  // need this because sometimes a last error was stuck and we couldn't enter the loop below

    WIN32_FIND_DATA dirdata;
    HANDLE hDirSearch = FindFirstFile("*.*", &dirdata);
    if ( hDirSearch == INVALID_HANDLE_VALUE )
    {
        getConsole().EOLnOO("ERROR: hDirSearch == INVALID_HANDLE_VALUE");
        return;
    }

    sFoundProfiles = PGENULL;
    sFoundProfilePlayerNames = PGENULL;
    while ( (GetLastError() != ERROR_NO_MORE_FILES) && (hDirSearch != INVALID_HANDLE_VALUE) )
    {
        string sCfgFileName;
        if ( (strcmp(dirdata.cFileName,".") != 0) && (strcmp(dirdata.cFileName,"..") != 0) )
        {
            if ( (dirdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
            {
                sCfgFileName = dirdata.cFileName;
                sCfgFileName += ".cfg";
                SetCurrentDirectory(dirdata.cFileName);
                WIN32_FIND_DATA filedata;
                HANDLE hFileSearch = FindFirstFile(sCfgFileName.c_str(), &filedata);
                if ( (GetLastError() != ERROR_NO_MORE_FILES) && (hFileSearch != INVALID_HANDLE_VALUE) )
                {
                    string sPlayerName;
                    // note: temporarily disabled file size checking ... decide later if it's needed
                    /*if ( (filedata.nFileSizeLow >= PGE_CFG_SETTINGSFILE_MIN_SIZE) &&
                         (filedata.nFileSizeLow <= PGE_CFG_SETTINGSFILE_MAX_SIZE) )*/
                    {   // checking file size, just in case ...
                        if ( getPlayerNameFromFile(sCfgFileName.c_str(), sPlayerName) )
                        {
                            nProfilesCount++; 
                            std::string** sNewFoundProfiles = PGENULL;
                            try
                            {
                                if ( sFoundProfiles )
                                {   // mimicing realloc, this to be changed to vector or something!
                                    sNewFoundProfiles = new string*[nProfilesCount]();
                                    memcpy(sNewFoundProfiles, sFoundProfiles, sizeof(string*)*nProfilesCount);
                                    delete[] sFoundProfiles;
                                    sFoundProfiles = sNewFoundProfiles;
                                }
                                else
                                {
                                    sFoundProfiles = new string*[nProfilesCount]();
                                }
                                sFoundProfiles[nProfilesCount-1] = new string(dirdata.cFileName);
                                if ( sFoundProfilePlayerNames )
                                {   // mimicing realloc, this to be changed to vector or something!
                                    std::string** sNewFoundProfilePlayerNames = new string*[nProfilesCount]();
                                    memcpy(sNewFoundProfilePlayerNames, sFoundProfilePlayerNames, sizeof(string*)*nProfilesCount);
                                    delete[] sFoundProfilePlayerNames;
                                    sFoundProfilePlayerNames = sNewFoundProfilePlayerNames;
                                }
                                else
                                {
                                    sFoundProfilePlayerNames = new string*[nProfilesCount]();
                                }
                                sFoundProfilePlayerNames[nProfilesCount-1] = new string(sPlayerName);
                                getConsole().OLn("added user %s ~ %s", dirdata.cFileName, sPlayerName.c_str());
                                if ( getProfile() != -1 )
                                {   // update current profile index that may change during this function
                                    if ( *(sFoundProfiles[nProfilesCount-1]) == sOriginalCurrentProfileUser )
                                        nActiveProfile = nProfilesCount-1;
                                }
                            } // try
                            catch (const std::bad_alloc&)
                            {
                                FindClose(hFileSearch);
                                FindClose(hDirSearch);
                                SetCurrentDirectory( pOrigCurrDirr );
                                if ( sFoundProfiles != NULL )
                                {
                                    for (int i = 0; i < nProfilesCount-1; i++)
                                    {
                                        delete sFoundProfiles[i];
                                    }
                                }
                                if ( sFoundProfilePlayerNames != NULL )
                                {
                                    for (int i = 0; i < nProfilesCount-1; i++)
                                    {
                                        delete sFoundProfilePlayerNames[i];
                                    }
                                }
                                delete[] sFoundProfiles;
                                delete[] sFoundProfilePlayerNames;
                                getConsole().EOLnOO("ERROR: memory allocation failure!");
                                return;
                            }
                        }
                    } // file size check
                }
                FindClose(hFileSearch);
                SetCurrentDirectory( sPathToProfiles.c_str() );
            } // is directory
        } // ., ..
        // find more directories ...
        FindNextFile(hDirSearch, &dirdata);
    }
    FindClose(hDirSearch);
    SetCurrentDirectory( pOrigCurrDirr );

    getConsole().SOLnOO("> done!");

} // LoadProfilesList()


/**
    Removes all cvariables.
*/
void PGESysCFG::ClearVars()
{
    m_vars.clear();
} // ClearVars()


/**
    Writes lines to an opened file.
    The stream must be opened.
    The stream must be closed outside of this function.

    @param f_cfg The opened file stream where the function will write to.
    @param sUser The user name of the config owner.
    @param sNick The nickname of the config owner.
                 If specified, only this cvar will be written to the stream, otherwise all.
                 Specify this only if this is a new profile being added and not yet current profile.
                 You can leave it empty if you want to save the current profile.
*/
void PGESysCFG::WriteConfiguration(ofstream& f_cfg, const string& sUser, const string& sNick)
{
    f_cfg << PGE_SYS_CFG_FILE_MAGIC_START << endl << endl;
    f_cfg << "# " << sUser << "'s settings file" << endl << endl;
    
    if ( sNick.empty() )
    {
        f_cfg << "####### GAMEPLAY #######" << endl << endl;
        f_cfg << "####### MOUSE #######" << endl << endl;
        f_cfg << "####### KEYBOARD #######" << endl << endl;
        f_cfg << "####### VIDEO #######" << endl << endl;
        f_cfg << "####### AUDIO #######" << endl << endl;
        f_cfg << "####### SERVER #######" << endl << endl;

        std::map<std::string, PGEcfgVariable>::const_iterator it;
        for (it = m_vars.begin(); it != m_vars.end(); it++)
            f_cfg << it->first << "=" << it->second.getAsString() << endl;
    }
    else
        f_cfg << PGE_SYS_CFG_PLAYER_NAME_CVAR << "=" << sNick << endl;

    f_cfg << endl;
} // WriteConfiguration()



