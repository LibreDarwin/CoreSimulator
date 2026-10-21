/*
 * sim_help.inc - per-subcommand usage texts for `simctl help <subcommand>`.
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Generated to mirror the reference implementation's output byte for byte.
 * Do not edit by hand.
 */

struct sim_help_entry {
	const char *name;
	const char *text;
};

static const char sim_help_text_addmedia[] =
"Add photos, live photos, videos, or contacts to the library "
"of a device.\nUsage: simctl addmedia <device> <path> [... <pa"
"th>]\n\nYou can specify multiple files including a mix of phot"
"os, videos, and contacts.\n\nYou can also specify multiple liv"
"e photos by providing the photo and video files. They will a"
"utomatically be discovered and imported correctly.\n\nContacts"
" support the vCard format.\n"
;

static const char sim_help_text_addphoto[] =
"Add photos to the photo library of a device.\nUsage: simctl a"
"ddphoto <device> <path> [... <path>]\n"
;

static const char sim_help_text_addvideo[] =
"Add videos to the photo library of a device.\nUsage: simctl a"
"ddvideo <device> <path> [... <path>]\n"
;

static const char sim_help_text_appinfo[] =
"Show information about an installed application.\nUsage: simc"
"tl appinfo <device> <bundle identifier>\n"
;

static const char sim_help_text_boot[] =
"Boot a device or device pair.\nUsage: simctl boot <device> [-"
"-arch=<arch>] [--disabledJob=<job>] [--enabledJob=<job>] [--"
"checked-allocations]\n\n\t--arch=<arch>           Specify the a"
"rchitecture to use when booting the simulator (eg: 'arm64' o"
"r 'x86_64')\n\t--disabledJob=<job>     Disables the given laun"
"chd job. Multiple jobs can be disabled by passing multiple f"
"lags.\n\t--enabledJob=<job>      Enables the given launchd job"
" when it would normally be disabled.\n\t                      "
"  Multiple jobs can be enabled by passing multiple flags.\n\t-"
"-checked-allocations   Spawn all processes during this boot "
"session with checked allocations enabled (if supported).\n\n\nI"
"f you want to set environment variables in the resulting env"
"ironment, set them in the calling environment with a SIMCTL_"
"CHILD_ prefix.\n"
;

static const char sim_help_text_bootstatus[] =
"Checks device boot status.\nUsage: simctl bootstatus <device>"
" [-bcd]\n\t-b       Boot the device if it isn't already booted"
".\n\t-d       Print data migration information, if it's availa"
"ble.\n\t-c       Continuously monitor boot status through mult"
"iple boot/shutdown cycles.\n\nMonitors the specified device an"
"d prints boot status information until the device finishes b"
"ooting. You can safely call this before you attempt to start"
" booting the device.\n\n"
;

static const char sim_help_text_clone[] =
"Clone an existing device.\nUsage: simctl clone <device> <new "
"name> [<destination device set>]\n"
;

static const char sim_help_text_create[] =
"Create a new device.\nUsage: simctl create <name> <device typ"
"e id> [<runtime id>]\n\n\t<device type id>    A valid available"
" device type. Find these by running \"xcrun simctl list devic"
"etypes\".\n\t                    Examples: (\"iPhone X\", \"com.ap"
"ple.CoreSimulator.SimDeviceType.iPhone-X\")\n\t<runtime id>    "
"    A valid and available runtime. Find these by running \"xc"
"run simctl list runtimes\".\n\t                    If no runtim"
"e is specified the newest runtime compatible with the device"
" type is chosen.\n\t                    Examples: (\"watchOS3\","
" \"watchOS3.2\", \"watchOS 3.2\", \"com.apple.CoreSimulator.SimRu"
"ntime.watchOS-3-2\",\n\t                               \"/Volume"
"s/path/to/Runtimes/watchOS 3.2.simruntime\")\n\n"
;

static const char sim_help_text_darwinup[] =
"Invoke darwinup to install a root for the specified simulato"
"r runtime.\nUsage: simctl darwinup [-fnsv] <platform> <comman"
"d> [<archive> ...]\n\noptions:\n\t-f         Force operation to "
"succeed at all costs.\n\t-n         Dry run; prints arguments "
"that would have been sent to darwinup.\n\t-s         Use the S"
"DK root path instead of the runtime root.\n\t-v         More v"
"erbose output.\n\n<platform> is the Simulator platform, i.e. i"
"phonesimulator, watchsimulator, appletvsimulator.\n(To target"
" a runtime disk image, specify the path to that runtime inst"
"ead. Invalid with -s).\n\ncommands:\n\tinstall    Install the ro"
"ots specified by one or more <archive> arguments.\n\tlist     "
"  List the installed roots, with optional filter using <arch"
"ive> arguments.\n\tuninstall  Remove the roots specified by on"
"e or more <archive> arguments.\n\n\tSee darwinup usage for addi"
"tional commands and more information.\n\nNOTE: If you install "
"or uninstall roots, you must manually update the dyld_sim sh"
"ared cache.\n      See simctl help legacyruntime for informat"
"ion about triggering a dyld_sim shared cache rebuild.\n\n"
;

static const char sim_help_text_delete[] =
"Delete specified devices, unavailable devices, or all device"
"s.\nUsage: simctl delete <device> [... <device n>] | unavaila"
"ble | all\n\nSpecifying unavailable will delete devices that a"
"re not supported by the current Xcode SDK.\n\n"
;

static const char sim_help_text_diagnose[] =
"Collect diagnostic information and logs.\nUsage: simctl diagn"
"ose [-bX] [--timeout=<duration>] [--output=<path>] [--no-arc"
"hive] [--all-logs] [--data-containers] [--udid=<device> [--u"
"did=<device> [...]]]\n\n\t-b                Do NOT show the res"
"ulting archive in a Finder window upon completion.\n\t-X      "
"          Run all diagnostics with no timeout. It ignores th"
"e --timeout value if it is specified.\n\t--timeout         Spe"
"cify a duration (in seconds) to wait for the log collection "
"before timeout.\n\t--output          Specify the output direct"
"ory.\n\t                  If not provided the temporary direct"
"ory is used.\n\t--no-archive      Do not create an archive, le"
"ave the collected files uncompressed.\n\t--all-logs        Inc"
"lude all device logs, even for non-booted devices.\n\t--data-c"
"ontainer  Include booted device(s) data directory.\n\t        "
"          Warning: May include private information, app data"
" containers,\n\t                  and increases the size of th"
"e archive!\n\t                  Default is NOT to collect the "
"data container.\n\t--udid            Collect diagnostics only "
"from the specified device. This option\n\t                  ma"
"y be specified more than once to consider multiple devices.\n"
"\t                  The --all-logs option causes all --udid o"
"ptions to be ignored.\n\nPlease provide simctl diagnose output"
" when filing bugs.\n\nNOTE: If there are booted devices only l"
"ogs from those devices are collected by default, otherwise -"
"-all-logs is assumed. More information can be collected from"
" a booted device so you should prefer leaving affected simul"
"ators booted when running simctl diagnose. \n\nNOTE: You can r"
"un 'simctl logverbose <device> enable' to enable verbose log"
"ging and reboot the device before reproducing the problem an"
"d running 'simctl diagnose'.\n\n"
;

static const char sim_help_text_erase[] =
"Erase a device's contents and settings.\nUsage: simctl erase "
"<device> [... <device n>] | all\n\nSpecifying all will erase a"
"ll existing devices.\n"
;

static const char sim_help_text_get_app_container[] =
"Print the path of the installed app's container\nUsage: simct"
"l get_app_container <device> <app bundle identifier> [<conta"
"iner>]\n\ncontainer   Optionally specify the container. Defaul"
"ts to app.\n  app                 The .app bundle\n  data     "
"           The application's data container\n  groups        "
"      The App Group containers\n  <group identifier>  A speci"
"fic App Group container\n\n\n"
;

static const char sim_help_text_getenv[] =
"Print an environment variable from a running device.\nUsage: "
"simctl getenv <device> <variable name>\n"
;

static const char sim_help_text_help[] =
"Prints the usage for a given subcommand.\nUsage: simctl help "
"<subcommand>\n"
;

static const char sim_help_text_icloud_sync[] =
"Trigger iCloud sync on a device.\nUsage: simctl icloud_sync <"
"device>\n"
;

static const char sim_help_text_install[] =
"Install an app on a device.\nUsage: simctl install <device> <"
"path>\n"
;

static const char sim_help_text_install_app_data[] =
"Install an xcappdata package to a device, replacing the curr"
"ent contents of the container.\nUsage: simctl install_app_dat"
"a <device> <path to xcappdata package>\n\t This will replace t"
"he current contents of the container. If the app is currentl"
"y running it will be terminated before the container is repl"
"aced.\n"
;

static const char sim_help_text_io[] =
"Set up a device IO operation.\nUsage: simctl io <device> <ope"
"ration> <arguments>\n\nCommon arguments:\n\t    --display    Spe"
"cifies which display to record. Accepts:\n\t                 -"
" Screen ID (eg: \"2\") \n\t                 - Screen name (eg: \""
"TVOut\")\n\t                 - Screen uniqueId (eg: \"PurpleTVOu"
"t\")\n\t                 - Port UUID (eg: \"D5293DAA-4D8C-453E-B"
"049-969FE81936A0\"\n\t                 - Alias (for compatibili"
"ty): \"internal\" or \"external\"\n\t                 Use 'simctl "
"io <device> enumerate' to list available screens and their I"
"Ds.\n\nSupported operations:\n\tenumerate [--poll]\n\t    Lists al"
"l available IO ports and descriptor states.\n\t    --poll     "
"  Poll after enumeration.\n\n\tpoll\n\t    Polls all available IO"
" ports for events.\n\n\trecordVideo [--codec=<codec>] [--displa"
"y=<display>] [--mask=<policy>] [--force] <file or url>\n\t    "
"Records the display to a QuickTime movie at the specified fi"
"le or url.\n\t    --codec      Specifies the codec type: \"h264"
"\" or \"hevc\". Default is \"hevc\".\n\n\t    --mask       For non-r"
"ectangular displays, handle the mask by policy:\n\t           "
"      ignored: The mask is ignored and the unmasked framebuf"
"fer is saved.\n\t                 alpha: Not supported, but re"
"tained for compatibility; the mask is rendered black.\n\t     "
"            black: The mask is rendered black.\n\n\t    --force"
"      Force the output file to be written to, even if the fi"
"le already exists.\n\n\tsimctl writes 'Recording started' to st"
"derr once the first video frame has been processed. Look for"
" this\n\tif you want to wait for the recording to start.\n\tSend"
" SIGINT (Control + C) to stop recording. simctl exits once t"
"he in-flight frames are processed\n\tand the video file is fin"
"alized.\n\n\tscreenshot [--type=<type>] [--display=<display>] ["
"--mask=<policy>] <file or url>\n\t    Saves a screenshot as a "
"PNG to the specified file or url(use \"-\" for stdout).\n\t    -"
"-type       Can be \"png\", \"tiff\", \"bmp\", \"gif\", \"jpeg\". Defa"
"ult is png.\n\n\t    --mask       For non-rectangular displays,"
" handle the mask by policy:\n\t                 ignored: The m"
"ask is ignored and the unmasked framebuffer is saved.\n\t     "
"            alpha: The mask is used as premultiplied alpha.\n"
"\t                 black: The mask is rendered black.\n\n\tscree"
"nConfig [--display=<display>] <subcommand> [arguments]\n\t    "
"Manage device screen configuration.\n\n\t    power <on|off>\n\t  "
"      Set the power state of a screen.\n\n\t    geometry <width"
">x<height>[@<scale>]\n\t        Change the geometry of a scree"
"n.\n\t        The width and height must be compatible with the"
" screen's current mode.\n\t        If @<scale> is specified, a"
" mode with that scale factor will be selected.\n\nExample:\n\tSa"
"ve a screenshot of the booted device to screenshot.png:\n\tsim"
"ctl io booted screenshot screenshot.png\n\n\n"
;

static const char sim_help_text_keyboard[] =
"Set the device's keyboard language.\nUsage: simctl keyboard <"
"device> <language>\n"
;

static const char sim_help_text_keychain[] =
"Manipulate a device's keychain\nUsage: simctl keychain <devic"
"e> <action> [arguments]\n\n    add-root-cert [path]\n        Ad"
"d a certificate to the trusted root store.\n\n    add-cert [pa"
"th]\n        Add a certificate to the keychain.\n\n    reset\n  "
"      Reset the keychain.\n\n\n"
;

static const char sim_help_text_launch[] =
"Launch an application by identifier on a device.\nUsage: simc"
"tl launch [-w | --wait-for-debugger] [-a <arch> | --arch=<ar"
"ch>] [--console|--console-pty] [--stdout=<path>] [--stderr=<"
"path>] [--terminate-running-process] [--checked-allocations]"
" <device> <app bundle identifier> [<argv 1> <argv 2> ... <ar"
"gv n>]\n\n\t--arch Specify an architecture (x86_64 or arm64) to"
" use when spawning a universal binary.  Requires runtime ver"
"sion 26 or newer.\n\t--console Block and print the application"
"'s stdout and stderr to the current terminal.\n\t\tSignals rece"
"ived by simctl are passed through to the application.\n\t\t(Can"
"not be combined with --stdout or --stderr)\n\t--console-pty Bl"
"ock and print the application's stdout and stderr to the cur"
"rent terminal via a PTY.\n\t\tSignals received by simctl are pa"
"ssed through to the application.\n\t\t(Cannot be combined with "
"--stdout or --stderr)\n\t--stdout=<path> Redirect the applicat"
"ion's standard output to a file.\n\t--stderr=<path> Redirect t"
"he application's standard error to a file.\n\t--terminate-runn"
"ing-process Terminate any running copy of the application.\n\t"
"--checked-allocations Launch the app with checked allocation"
"s enabled (if supported).\n\t\tNote: Log output is often direct"
"ed to stderr, not stdout.\n\nIf you want to set environment va"
"riables in the resulting environment, set them in the callin"
"g environment with a SIMCTL_CHILD_ prefix.\n"
;

static const char sim_help_text_legacyruntime[] =
"Locate, copy, mount, and unmount simulator runtime disk imag"
"es.\nUsage: simctl legacyruntime <operation> <arguments>\n\nSup"
"ported Operations:\n    mount <train-build> [--shadow <path>]"
" [--internal | --customer] [--writable] [--no-mount] [--no-c"
"opy] [--print-only] [--use-existing <path>] [--max-cache-siz"
"e <size>] [--cache-path <path>]\n\tLocate a Simulator Runtime "
"for the given train and build, copy the disk image locally, "
"then mount it to make the runtime available to CoreSimulator"
".\n\n\t--internal\n\t     (Default) Select the internal runtime.\n"
"\t--customer\n\t     Select the customer (public) runtime.\n\t--s"
"hadow <path>\n\t     Manually specify a different shadow image"
". The same base image can be mounted multiple\n\t     times wi"
"th different shadow files so modifications to one mount path"
" are not visible to others.\n\t--no-sparse\n\t     By default im"
"ages are converted to a sparse image, then the partition siz"
"e is set to 100GB.\n\t     This leaves plenty of room to insta"
"ll roots without wasting space (sparse images grow as needed"
").\n\t     This flag disables this behavior: the image will ha"
"ve the original format and partition size.\n\t     NOTE: You d"
"on't want this.\n\t--writable\n\t     When mounting the disk ima"
"ge, do not use a shadow file: directly mount the image as wr"
"itable.\n\t     NOTE: You don't want this.\n\t--use-existing <pa"
"th>\n\t     By default images are downloaded from NFA. If this"
" is passed in, the image specified at the provided path will"
" be used instead of downloading one.\n\t--no-mount\n\t     Copy "
"the image to the cache (if not present), but do not actually"
" mount it. Prints the path of the image as the last line of "
"output.\n\t--print-only\n\t     Print the path to the image on t"
"he network but take no other action.\n\n\n    unmount <train-bu"
"ild> [--force]\n\tUnmount the Simulator Runtime if it is mount"
"ed. Any booted simulators will be shut down first.\n\tThe alia"
"s 'all' will unmount all runtime disk images known to CoreSi"
"mulator.\n\n\t--force\n\t     Force-unmount even if the disk is b"
"usy.\n\n\n    cache [list | clear [--dry-run]] [--cache-path <p"
"ath>]\n\tlist\n\t     Lists cached runtime disk images\n\n\tclear\n\t"
"     Deletes all simulator runtime disk images (excludes run"
"times bundled with Xcode or downloaded runtimes in /Library)"
".\n\t     Any mounted runtimes are unmounted before deletion.\n"
"\n\t--dry-run\n\t     Run the clear command but print what would"
" have been deleted, without actually deleting anything.\n\n\n  "
"  pin <device> [--latest | --build <train-version> | --path "
"<path to runtime>]\n\tPin a device to use a specific runtime.\n"
"\n\t--system\n\t     (Default) Use the system policy. Customer O"
"S: match SDK build if possible. All others use latest.\n\t--ne"
"west\n\t     Always use the newest build matching the semantic"
" version.\n\t--build <train-version>\n\t     Pin the device to u"
"se a specific build of the runtime. If multiple runtimes mat"
"ch one is randomly chosen.\n\t--path <path to runtime>\n\t     P"
"in the device to use a specific runtime by path.\n\n\n    info "
"[path] [<platform> | <train-build>]\n\tpath\n\t     Lists the pa"
"th to the runtime (the path may contain spaces!)\n\t     If no"
" platform or train-build is specified then the iOS runtime b"
"undled with the selected Xcode is chosen.\n\t     Platform may"
" be iOS, tvOS, or watchOS to select the relevant Xcode-bundl"
"ed runtime.\n\n\n    update_dyld_shared_cache (<platform> | <tr"
"ain-build> | <path>)\n\tUpdate the dyld shared cache for a spe"
"cific runtime.\n\n\n    remove_dyld_shared_cache (<platform> | "
"<train-build>)\n\tRemoves the dyld shared cache for a specific"
" runtime.\n\n\tplatform | train-build\n\t     Platform may be iOS"
", tvOS, or watchOS to select the relevant Xcode-bundled runt"
"ime.\n\t     Otherwise the value is assumed to be a train-buil"
"d and the relevant mounted disk image is used.\n\t     If usin"
"g manually-mounted images or multiple mount points, pass the"
" path to the mountpoint.\n\n\t     The dyld_sim shared caches a"
"re stored in ~/Library/Developer/CoreSimulator/Caches.\n\t    "
" The cache is tied to the specific build of the runtime and "
"the host macOS.\n\t     If you upgrade macOS, the caches will "
"be re-built automatically by CoreSimulator in the background"
"\n\t     at low priority. In all cases, caches for a macOS tha"
"t isn't the current macOS will be automatically scheduled fo"
"r deletion.\n\n\nA <train-version> means a train name (case-sen"
"sitive) and build number, such as 'Example10A101'. The only "
"supported alias is 'Current', eg 'CurrentExample'.\n\nUnless a"
" simulated device specifies an explicit build, it will boot "
"with the newest build of the given platform and matching maj"
"or.minor.revision.\nFor example if a device was created with "
"iOS 12.0, it will prefer the newest build of iOS 12.0 it can"
" find. You can have as many builds of iOS 12 mounted\nas you "
"like, but only devices pinned to a runtime will use any othe"
"r build besides the latest. To change an existing device's p"
"inning\nuse pin (see above).\nOther commands that take a runti"
"me identifier (such as simctl darwinup) also accept a <train"
"-version> or path to the disk image mount point, so long as "
"you specify\nenough information to uniquely identify a single"
" runtime. If the specifier is ambiguous simctl will error ra"
"ther than making any assumptions.\n\nBy default up to 50 GB of"
" runtime images are cached locally for greatly improved perf"
"ormance. Old images are deleted when the cache grows too lar"
"ge.\nThe cache-path and max-cache-size default values can be "
"modified by setting user defaults:\n  defaults write com.appl"
"e.CoreSimulator RuntimeImageCachePath '<path>'\n  defaults wr"
"ite com.apple.CoreSimulator RuntimeImageCacheMaxSizeGB 10\nAl"
"l commands accept --cache-path <path> to override the user d"
"efault.\nOnly the mount command automatically prunes the cach"
"e and it accepts --max-cache-size <size in GB> to override t"
"he user default.\n\nThe default paths searched on the network "
"can be changed or expanded. This should be to the Images sub"
"directory:\n  defaults write com.apple.CoreSimulator RuntimeI"
"mageSearchPaths -array-add '/MountPoint/Team/Images'\n\nCoreSi"
"mulator monitors for disk mount and unmount notifications, s"
"o you can manually locate the image and mount it if simctl d"
"oes not meet your needs, eg:\n  hdiutil attach <image> -mount"
"point /Volumes/Example10A101 -shadow\n  hdiutil detach /Volum"
"es/Example10A101\n\n\n"
;

static const char sim_help_text_listapps[] =
"Show the installed applications.\nUsage: simctl listapps <dev"
"ice>\n"
;

static const char sim_help_text_location[] =
"Control a device's simulated location\nUsage: simctl location"
" <device> <action> [arguments]\n\n    list\n        List availa"
"ble simulation scenarios.\n\n    clear\n        Stop any runnin"
"g scenario and clear any simulated location.\n\n    set <lat1>"
",<lon1>\n        Set the location to a specific latitude and "
"longitude.\n\n    run <scenario>\n        Run a simulated locat"
"ion scenario (use the list action to get a list of scenarios"
").\n\n    start [--speed=<meters/sec>] [--distance=<meters per"
" second>|--interval=<seconds>] <lat1>,<lon1> <latN>,<lonN>.."
".\n        Set the location to a series of waypoints specifie"
"d as 'lat,lon' pairs, interpolating between them over time.\n"
"        At least two waypoints are required. Use '-' to read"
" waypoints from stdin, one waypoint per line.\n\n        Speed"
" specifies how quickly to move between waypoints in meters p"
"er second. If not specified 20m/s is used.\n\n        The syst"
"em will issue location updates along the path between each p"
"air of waypoints. Use distance or interval to \n        contr"
"ol how often those updates are issued. Distance will issue a"
"n update every <meters> travelled without regard\n        for"
" the time between updates. Interval will issue updates at fi"
"xed times without regard for how much\n        the location m"
"oves between updates.\n        If neither are specified an in"
"terval of 1.0 seconds is used. If both are specified the sys"
"tem picks ones.\n\n        Example simulating a direct line be"
"tween San Francisco and New York City, with updates every km"
":\n          start --distance=1000 --speed=260 37.629538,-122"
".395733 40.628083,-73.768254\n\nLatitude and longitude pairs m"
"ust be specified using '.' as the decimal separator and ',' "
"as the field separator.\n\n\n"
;

static const char sim_help_text_logverbose[] =
"enable or disable verbose logging for a device\nUsage: simctl"
" logverbose [<device>] (enable | disable)\n\t<device>         "
"The device. If not provided all booted devices are affected."
"\n\tenable           Enables verbose logging\n\tdisable         "
" Disables verbose logging\n\nNOTE: You may need to reboot the "
"affected device before logging changes will be effective.\n\n"
;

static const char sim_help_text_monitor[] =
"List notifications as they arrive.\nUsage: simctl monitor \n"
;

static const char sim_help_text_notify_get_state[] =
"Get the state value of a darwin notification on a device.\nUs"
"age: simctl notify_get_state <device> <notification name>\n"
;

static const char sim_help_text_notify_post[] =
"Post a darwin notification on a device.\nUsage: simctl notify"
"_post <device> <notification name>\n"
;

static const char sim_help_text_notify_set_state[] =
"Set the state value of a darwin notification on a device.\nUs"
"age: simctl notify_set_state <device> <notification name> <s"
"tate>\n"
;

static const char sim_help_text_openurl[] =
"Open a URL in a device.\nUsage: simctl openurl <device> <URL>"
"\n"
;

static const char sim_help_text_pair[] =
"Create a new watch and phone pair.\nUsage: simctl pair <watch"
" device> <phone device>\n"
;

static const char sim_help_text_pair_activate[] =
"Set a given pair as active.\nUsage: simctl pair_activate <pai"
"r>\n"
;

static const char sim_help_text_pbcopy[] =
"Copy standard input onto the device pasteboard.\nUsage: simct"
"l pbcopy [-v] <device>\n"
;

static const char sim_help_text_pbinfo[] =
"Print information about the device's pasteboard\nUsage: simct"
"l pbinfo <device or \"host\">\n"
;

static const char sim_help_text_pbpaste[] =
"Print the contents of the device's pasteboard to standard ou"
"tput.\nUsage: simctl pbpaste [-v] <device>\n"
;

static const char sim_help_text_pbsync[] =
"Sync the pasteboard content from one pasteboard to another.\n"
"Usage: simctl pbsync [-pv] <source device or \"host\"> <destin"
"ation device or \"host\">\n\n -p causes simctl to use promise da"
"ta for secondary types.  simctl will continue to run to prov"
"ide that promise data until something else replaces it on th"
"e pasteboard.\n"
;

static const char sim_help_text_personalization[] =
"Provides utility when working with personalization manifests"
"\nUsage: simctl personalization <action> [arguments]\n    pers"
"onalize <identifier> - Personalizes the specified runtime ge"
"nerating a new personalization manifest.\n    remove-manifest"
" <filename> - Removes the specified personalization manifest"
" file from /Library/Developer/CoreSimulator/Cryptex/Personal"
"ization\n    remove-all-manifests - Removes all of the person"
"alization manifests from /Library/Developer/CoreSimulator/Cr"
"yptex/Personalization\n    remove-personalization <id> - Remo"
"ves the personalization for the runtime and remounts it.\n   "
" revoke-manifests - Revokes personalization manifests requir"
"ing all runtime cryptex mounts to be re-personalized.\n    sc"
"an-and-personalize - Scans for any simulator runtimes that m"
"ay need to have their personalization manifests updated, upd"
"ates them, and remounts them as a cryptex.\n\n"
;

static const char sim_help_text_privacy[] =
"Grant, revoke, or reset privacy and permissions\nUsage: simct"
"l privacy <device> <action> <service> [<bundle identifier>]\n"
"\n\taction\n\t     The action to take:\n\t         grant - Grant a"
"ccess without prompting. Requires bundle identifier.\n\t      "
"   revoke - Revoke access, denying all use of the service. R"
"equires bundle identifier.\n\t         reset - Reset access, p"
"rompting on next use. Bundle identifier optional.\n\t     Some"
" permission changes will terminate the application if runnin"
"g.\n\tservice\n\t     The service:\n\t         all - Apply the act"
"ion to all services.\n\t         calendar - Allow access to ca"
"lendar.\n\t         contacts-limited - Allow access to basic c"
"ontact info.\n\t         contacts - Allow access to full conta"
"ct details.\n\t         location - Allow access to location se"
"rvices when app is in use.\n\t         location-always - Allow"
" access to location services at all times.\n\t         photos-"
"add - Allow adding photos to the photo library.\n\t         ph"
"otos - Allow full access to the photo library.\n\t         med"
"ia-library - Allow access to the media library.\n\t         mi"
"crophone - Allow access to audio input.\n\t         motion - A"
"llow access to motion and fitness data.\n\t         reminders "
"- Allow access to reminders.\n\t         siri - Allow use of t"
"he app with Siri.\n\tbundle identifier\n\t     The bundle identi"
"fier of the target application.\n\nExamples:\n\treset all permis"
"sions: privacy <device> reset all\n\tgrant test host photo per"
"missions: privacy <device> grant photos com.example.app.test"
"-host\n\nWarning:\nNormally applications must have valid Info.p"
"list usage description keys and follow the API guidelines to"
" request access to services. Using this command to bypass th"
"ose requirements can mask bugs.\n\n\n"
;

static const char sim_help_text_push[] =
"Send a simulated push notification\nUsage: simctl push <devic"
"e> [<bundle identifier>] (<json file> | -)\n\n\tbundle identifi"
"er\n\t     The bundle identifier of the target application\n\t  "
"   If the payload file contains a 'Simulator Target Bundle' "
"top-level key this parameter may be omitted.\n\t     If both a"
"re provided this argument will override the value from the p"
"ayload.\n\tjson file\n\t     Path to a JSON payload or '-' to re"
"ad from stdin. The payload must:\n\t       - Contain an object"
" at the top level.\n\t       - Contain an 'aps' key with valid"
" Apple Push Notification values.\n\t       - Be 4096 bytes or "
"less.\n\nOnly application remote push notifications are suppor"
"ted. VoIP, Complication, File Provider, and other types are "
"not supported.\n\n\n"
;

static const char sim_help_text_register[] =
"Register a service from one bootstrap into another.\nUsage: s"
"imctl register <source device or \"host\">/<service> <destinat"
"ion device>/<service>\n"
;

static const char sim_help_text_rename[] =
"Rename a device.\nUsage: simctl rename <device> <name>\n"
;

static const char sim_help_text_reset[] =
"Reset a given archive or database.\nUsage: simctl reset <devi"
"ce> <subsystem>\n\nSupported Subsystems:\nlaunch_services\n\n"
;

static const char sim_help_text_shutdown[] =
"Shutdown a device.\nUsage: simctl shutdown <device> | all\n\nSp"
"ecifying all will shut down all running devices\n"
;

static const char sim_help_text_spawn[] =
"Spawn a process by executing a given executable on a device."
"\nUsage: simctl spawn [-w | --wait-for-debugger] [-s | --stan"
"dalone] [-a <arch> | --arch=<arch>] [--checked-allocations] "
"<device> <path to executable> [<argv 1> <argv 2> ... <argv n"
">]\n-w    Start the process suspended.\n-s    Use a NULL mach "
"bootstrap port, meaning the process cannot connect to other "
"services and does not require a booted device. (deprecated)\n"
"-a    Specify an architecture (x86_64 or arm64) to use when "
"spawning a universal binary.\n--checked-allocations Spawns th"
"e process with checked allocations enabled (if supported).\n\n"
"The path to the executable is searched using the following r"
"ules:\n   <path> contains no / characters: search the device'"
"s $PATH. This is similar to how most shells work, but search"
"es the device's path instead of the host's path.\n   <path> s"
"tarts with /: Assume a literal path to the binary. This must"
" start from the host's root.\n   <path> contains non-leading "
"/ characters: search relative to the current directory first"
", then relative to the device's $SIMULATOR_ROOT.\n\nIf you wan"
"t to set environment variables in the resulting environment,"
" set them in the calling environment with a SIMCTL_CHILD_ pr"
"efix.\n"
;

static const char sim_help_text_status_bar[] =
"Set or clear status bar overrides\nUsage: simctl status_bar <"
"device> [list | clear | override <override arguments>]\n\nSupp"
"orted Operations:\n    list\n\tList existing overrides.\n\n    cl"
"ear\n\tClear all existing status bar overrides.\n\n    override "
"<override arguments>\n\tSet status bar override values, accord"
"ing to these flags.\n\tYou may specify any combination of thes"
"e flags (at least one is required):\n\n\t--time <string>\n\t     "
"Set the date or time to a fixed value.\n\t     If the string i"
"s a valid ISO date string it will also set the date on relev"
"ant devices.\n\t--dataNetwork <dataNetworkType>\n\t     If speci"
"fied must be one of 'hide', 'wifi', '3g', '4g', 'lte', 'lte-"
"a', 'lte+', '5g', '5g+', '5g-uwb', or '5g-uc'.\n\t--wifiMode <"
"mode>\n\t     If specified must be one of 'searching', 'failed"
"', or 'active'.\n\t--wifiBars <int>\n\t     If specified must be"
" 0-3.\n\t--cellularMode <mode>\n\t     If specified must be one "
"of 'notSupported', 'searching', 'failed', or 'active'.\n\t--ce"
"llularBars <int>\n\t     If specified must be 0-4.\n\t--operator"
"Name <string>\n\t     Set the cellular operator/carrier name. "
"Use '' for the empty string.\n\t--batteryState <state>\n\t     I"
"f specified must be one of 'charging', 'charged', or 'discha"
"rging'.\n\t--batteryLevel <int>\n\t     If specified must be 0-1"
"00.\n\n\n"
;

static const char sim_help_text_terminate[] =
"Terminate an application by identifier on a device.\nUsage: s"
"imctl terminate <device> <app bundle identifier>\n"
;

static const char sim_help_text_ui[] =
"Get or Set UI options\nUsage: simctl ui <device> <option> [<a"
"rguments>]\n\nSupported Options:\n    appearance\n\tWhen invoked "
"without arguments prints the current user interface appearan"
"ce style:\n\t     light\n\t     The Light appearance style.\n\t   "
"  dark\n\t     The Dark appearance style.\n\t     unsupported\n\t "
"    The platform or runtime version do not support appearanc"
"e styles.\n\t     unknown\n\t     The current style is unknown o"
"r there was an error detecting it.\n\n    appearance [light | "
"dark]\n\tSet the user interface apperance style to light or da"
"rk.\n\n    increase_contrast\n\tWhen invoked without arguments p"
"rints whether the Increase Contrast mode is currently enable"
"d:\n\t     enabled\n\t     Increase Contrast is enabled.\n\t     d"
"isabled\n\t     Increase Contrast is disabled.\n\t     unsupport"
"ed\n\t     The platform or runtime version do not support the "
"Increase Contrast setting.\n\t     unknown\n\t     The current s"
"etting is unknown or there was an error detecting it.\n\n    i"
"ncrease_contrast [enabled | disabled]\n\tEnable or disable Inc"
"rease Contrast mode.\n\n    content_size\n\tWhen invoked without"
" arguments prints the current preferred content size categor"
"y, from the following possible values:\n\t     Standard sizes:"
" extra-small, small, medium, large, extra-large, extra-extra"
"-large, extra-extra-extra-large.\n\t     Extended range sizes:"
" accessibility-medium, accessibility-large, accessibility-ex"
"tra-large, accessibility-extra-extra-large, accessibility-ex"
"tra-extra-extra-large.\n\t     Other values: unknown, unsuppor"
"ted.\n\n    content_size [increment | decrement | desired_size"
"]\n\tSet the preferred content size category. If 'increment' o"
"r 'decrement' is passed, the preferred content size will bes"
"et to the category one increment larger or smaller than the "
"currently set value respectively.\n\n\n"
;

static const char sim_help_text_uninstall[] =
"Uninstall an app from a device.\nUsage: simctl uninstall <dev"
"ice> <app bundle identifier>\n"
;

static const char sim_help_text_unpair[] =
"Unpair a watch and phone pair.\nUsage: simctl unpair <pair UU"
"ID>\n"
;

static const char sim_help_text_unregister[] =
"Unregister a service from a device's bootstrap.\nUsage: simct"
"l unregister <device> <service>\n"
;

static const char sim_help_text_upgrade[] =
"Upgrade a device to a newer runtime.\nUsage: simctl upgrade <"
"device> <runtime id>\n"
;

static const struct sim_help_entry sim_help_entries[] = {
	{ "addmedia", sim_help_text_addmedia },
	{ "addphoto", sim_help_text_addphoto },
	{ "addvideo", sim_help_text_addvideo },
	{ "appinfo", sim_help_text_appinfo },
	{ "boot", sim_help_text_boot },
	{ "bootstatus", sim_help_text_bootstatus },
	{ "clone", sim_help_text_clone },
	{ "create", sim_help_text_create },
	{ "darwinup", sim_help_text_darwinup },
	{ "delete", sim_help_text_delete },
	{ "diagnose", sim_help_text_diagnose },
	{ "erase", sim_help_text_erase },
	{ "get_app_container", sim_help_text_get_app_container },
	{ "getenv", sim_help_text_getenv },
	{ "help", sim_help_text_help },
	{ "icloud_sync", sim_help_text_icloud_sync },
	{ "install", sim_help_text_install },
	{ "install_app_data", sim_help_text_install_app_data },
	{ "io", sim_help_text_io },
	{ "keyboard", sim_help_text_keyboard },
	{ "keychain", sim_help_text_keychain },
	{ "launch", sim_help_text_launch },
	{ "legacyruntime", sim_help_text_legacyruntime },
	{ "listapps", sim_help_text_listapps },
	{ "location", sim_help_text_location },
	{ "logverbose", sim_help_text_logverbose },
	{ "monitor", sim_help_text_monitor },
	{ "notify_get_state", sim_help_text_notify_get_state },
	{ "notify_post", sim_help_text_notify_post },
	{ "notify_set_state", sim_help_text_notify_set_state },
	{ "openurl", sim_help_text_openurl },
	{ "pair", sim_help_text_pair },
	{ "pair_activate", sim_help_text_pair_activate },
	{ "pbcopy", sim_help_text_pbcopy },
	{ "pbinfo", sim_help_text_pbinfo },
	{ "pbpaste", sim_help_text_pbpaste },
	{ "pbsync", sim_help_text_pbsync },
	{ "personalization", sim_help_text_personalization },
	{ "privacy", sim_help_text_privacy },
	{ "push", sim_help_text_push },
	{ "register", sim_help_text_register },
	{ "rename", sim_help_text_rename },
	{ "reset", sim_help_text_reset },
	{ "shutdown", sim_help_text_shutdown },
	{ "spawn", sim_help_text_spawn },
	{ "status_bar", sim_help_text_status_bar },
	{ "terminate", sim_help_text_terminate },
	{ "ui", sim_help_text_ui },
	{ "uninstall", sim_help_text_uninstall },
	{ "unpair", sim_help_text_unpair },
	{ "unregister", sim_help_text_unregister },
	{ "upgrade", sim_help_text_upgrade },
};

#define SIM_HELP_ENTRY_COUNT \
	(sizeof(sim_help_entries) / sizeof(sim_help_entries[0]))

static const char *
sim_help_lookup(const char *name)
{
	size_t i;

	for (i = 0; i < SIM_HELP_ENTRY_COUNT; i++) {
		if (strcmp(sim_help_entries[i].name, name) == 0)
			return sim_help_entries[i].text;
	}
	return NULL;
}
