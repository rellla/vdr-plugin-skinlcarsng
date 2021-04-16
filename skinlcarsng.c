#include <getopt.h>
#include <vdr/plugin.h>
#include "config.h"
#include "setup.h"
#include "lcarsng.h"

#if defined(APIVERSNUM) && APIVERSNUM < 10734
#error "VDR-1.7.34 API version or greater is required!"
#endif

static const char *VERSION        = "0.3.4";
static const char *DESCRIPTION    = "skin lcarsng";

class cPluginLCARSNG : public cPlugin {
    private:
        cLCARSNG *lcarsng;
    public:
        cPluginLCARSNG(void);
        virtual ~cPluginLCARSNG();
        virtual const char *Version(void) { return VERSION; }
        virtual const char *Description(void) { return DESCRIPTION; }
        virtual const char *CommandLineHelp(void);
        virtual bool ProcessArgs(int argc, char *argv[]);
        virtual bool Initialize(void);
        virtual bool Start(void);
        virtual void Stop(void);
        virtual void Housekeeping(void);
        virtual void MainThreadHook(void);
        virtual cString Active(void);
        virtual time_t WakeupTime(void);
        virtual const char *MainMenuEntry(void) {return NULL;}
        virtual cOsdObject *MainMenuAction(void);
        virtual cMenuSetupPage *SetupMenu(void);
        virtual bool SetupParse(const char *Name, const char *Value);
        virtual bool Service(const char *Id, void *Data = NULL);
        virtual const char **SVDRPHelpPages(void);
        virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
};

cPluginLCARSNG::cPluginLCARSNG(void) {
    lcarsng = NULL;
}

cPluginLCARSNG::~cPluginLCARSNG() {
}

const char *cPluginLCARSNG::CommandLineHelp(void) {
    return NULL; 
}

bool cPluginLCARSNG::ProcessArgs(int argc, char *argv[]) {
    return true;
}

bool cPluginLCARSNG::Initialize(void) {
    return true;
}

bool cPluginLCARSNG::Start(void) {
    if (!cOsdProvider::SupportsTrueColor()) {
        esyslog("skinlcarsng: No TrueColor OSD found! Aborting!");
        return false;
    } else
        dsyslog("skinlcarsng: TrueColor OSD found");
    cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
    lcarsng = new cLCARSNG;
    return lcarsng;
}

void cPluginLCARSNG::Stop(void) {
}

void cPluginLCARSNG::Housekeeping(void) {
}

void cPluginLCARSNG::MainThreadHook(void) {
}

cString cPluginLCARSNG::Active(void) {
    return NULL;
}

time_t cPluginLCARSNG::WakeupTime(void) {
    return 0;
}

cOsdObject *cPluginLCARSNG::MainMenuAction(void) {
    return NULL;
}

cMenuSetupPage *cPluginLCARSNG::SetupMenu(void) {
    return new cLCARSNGSetup();
}

bool cPluginLCARSNG::SetupParse(const char *Name, const char *Value) {
    return Config.SetupParse(Name, Value);
}

bool cPluginLCARSNG::Service(const char *Id, void *Data) {
    return false;
}

const char **cPluginLCARSNG::SVDRPHelpPages(void) {
    return NULL;
}

cString cPluginLCARSNG::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode) {
    return NULL;
}

VDRPLUGINCREATOR(cPluginLCARSNG); // Don't touch this!
