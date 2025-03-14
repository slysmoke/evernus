# Changelog

All notable changes to this project will be documented in this file.

## [4.53] - 2025-03-14

### 💼 Fixes


- Custom  FPC data validation
- FIX Sale Tax
- Release note link
- Makrket analysis column order when copying selected rows to clipboard
- Market analysis volume to INT and volume filter changed from avg. to median.
- Changed external order column Imported to  Last modified
- Added item ID comlumn
- Region details widget open as maximized window
- Chage lookup on evemarket.gitlab.io to evemarketbrowser.com



## [4.52] - 2025-01-31

### 🐛 Bug Fixes

- Fix: Fixed white window on startup
fix: Margin tools no longer resize the main window
style: Added donation button
- Fix: Fixed white window on startup
fix: Margin tools no longer resize the main window
style: Added donation button
- Removed info level messages from logs
- Added data validation in Fast Price Copy

### 💼 Other

- Skip chore(pr) commits

### ⚡ Performance

- Added caching for ESI

### 🎨 Styling

- Updated Sample data in Margin tools
- Changed title bar to dark

## [4.51] - 2024-07-29

### 💼 Other

- Release v4.5
- Sales tax update

## [4.4] - 2024-07-14

### 💼 Other

- Release v4.3
- Remove Ancestry
- Add db migration
- Change eveMarketerUrl

## [4.1] - 2023-07-15

### 🐛 Bug Fixes

- Fix

### 💼 Other

- Release v4.1
- Release v4.1
- Release v4.1
- Release v4.1

## [V4.0] - 2023-04-02

### 🐛 Bug Fixes

- Fixed task event propagation order
- Fixed key id deserialization
- Fixed import skills setting
- Fixed obsolete price deletion
- Fixed unintentinal foreign key cascade
- Fixed bind value order
- Fixed id for db
- Fixed time spec
- Fixed portrait cache location
- Fixed spin box initial state
- Fixed cache timers
- Fixed parsed time spec
- Fixed snapshot primary key
- Fixed closing on lost focus
- Fixed duplicate update timers
- Fixed buy price
- Fixed initial sorting
- Fixed last seen setting on order import
- Fixed syntax error
- Fixed clearing filter widget
- Fixed invalid table name
- Fixes for lupdate parser
- Fixed icon
- Fixed invalid timer age
- Fixes #4
- Fixed too late cache invalidation
- Fixed translation
- Fixed connections
- Fixed screwed migration logic
- Fixed old fulfilled orders staying changed forever
- Fix for potential 0.3->0.4 bad db update
- Fixed monospace font
- Fixed db rows parsing
- Fixed cache order import
- Fixed overwriting by obsolete orders
- Fix for VS release compiler bug
- Fixed empty script error message
- Fixed eve-marketdata.com price request overflow
- Fixed making asset snapshot without a character
- Fixed model data reset
- Fix for corporation transaction fetching
- Fix for corporation journal fetching
- Fixed unintentional searching through old api cache
- Fix for missing columns on upgrade
- Fixed corp web price import
- Fixed margin tool starting minimized
- Fixed not closing contract window
- Fixed margin tool opening from IGB
- Fixed eta calculation
- Fixed static analysis warnings
- Fixed corp market owner from logs
- Fixed sorting by eta

fixes #28
- Fixed corp order import
- Fixed setting key typo
- Fixed not-updating orders
- Fixed price difference %

refs #34
- Fixed number format in advanced statistics
- Fixed header state restoration

refs #34
- Fixed corp wallet plot

fixes #36
- Fixed size
- Fixed macro to string
- Fixed dropbox datetime parsing
- Fixed upload after no download
- Fixed station-only competitor order check
- Fixed empty buy order caching
- Fixed order value doubling
- Fixing doubled historic values
- Fixed sync with non-english locales
- Fixed profit column in archived orders
- Fixed not performing close actions on quiting via margin tool
- Fix for occasional invalid eve dt format
- Fixed scurity filter
- Fixed market browser total cost calculation
- Fixed crash with unwanted event loop
- Fix for multilevel assets
- Fixed eve-tq login url
- Fixed exception handling
- Fixed fpc shortcut logic
- Fixed error handling
- Fixed precision issues
- Fixed alignment
- Fixed BPC pricing in assets
- Fixed invalid auth errors

fixes #55
- Fixed exception propagation
- Fixed sqlite max param limit
- Fixed line endings
- Fixed error handlig which could cause a deadlock

refs #76
- Fixed error aggregation
- Fixed code style
- Fixes deadlock on invalid token/client errors

refs #76
- Fixed distance calculation

fixes #78
- Fixed closing timed out replies
- Fix for overwriting cloud db with a blank one
- Fixed uploading sync db
- Fixed not counting top items in totals calculation
- Fix for vs 2015 generic lambda parameter bug
- Fixed maintenance tool path in updater
- Fixed closing timed out replies
- Fix for translation parsing
- Fixed nan margin
- Fix for bound value overflow

fixes #150
- Fixed custom broker's fee updating
- Fixed error handling
- Fixed external order json parsing
- Fixed borked merge
- Fixed broker fee scale

fixes #161
- Fixed margin calculation + recalculating on use skill change

refs #167
- Fix install commands in CMakeLists.txt to use CMAKE_INSTALL_PREFIX
- Fixed auth for citadel order import
- Fixed crash on pref change
- Fixes for Qt 5.8 on Mac
- Fixed taxes when not using limit orders
- Fixed avg calculation (gaps in history)
- Fixed occasional nans
- Fixed fetching reprocessing info from db
- Fixed initial region text
- Fixed windows build
- Fixed saving header state for tree views
- Fixed median calculation
- Fixed log path
- Fixed race
- Fixed setting eve.db path from generic data location
- Fixed multi order deletion (for real this time)
- Fixup includes in QMacPasteboardMimeUnicodeText
- Fix 'a' key hijack on mac with FPC

[Cause] When shortcut was unset qxtglobalshortcut would register it as 0.
        (Disabling it would only early return instead of unregister) then even
        when set to something else it wouldn't unregister.
[Solution] Allow to unset manually and remove 0 check on MacOS.

It will throw some warnings but it's fine otherwise.
- Fixed linux breakpad name
- Fixed station axis
- Fixed major version updates while skipping minor versions
- Fixed setting proper setting based on button role
- Fixed synchronization

fixes #290
fixes #291
fixes #289
- Fixed crash when deleting orders
- Fixed type selection
- Fixed rounding error
- Fixed reading back skills
- Fixed race in generic names

fixes #310
- Fixed updating connection line y
- Fix buildbreak on MacOS
- Fix breakpad client framework finding on mac

IMPORTANT changes to Breakpad.xcodeproj
This also requires changing install_name/"Installation Directory" in xcode to
@rpath for this to work. Might also require changing copied resources source
from debug dir to release dir.

To properly deploy the app using macdeployqt in src/client/mac/build create a
link named lib pointing to Release dir:
ln -s Release lib
(macdeployqt has hardcoded last directory to lib).
- Fixed dockerfile for latest versions
- Fixed stack overflow
- Fixed library path
- Fixed setting original id
- Fixed pk
- Fixed typo
- Fixed crash on closing sso widget after error
- Fixes for latest base image
- Fixed handling of sell prices
- Fixed crash when no data is selected
- Fixed crash on removing invalid character

fixes #356
- Fixed sending unauthed json data
- Fixed importing price calculations
- Fixed build flags
- Fixed fetching contract items for corp

refs #414
- Fixed crash on char removal
- Fixed possible crash when setting nullptr label text
- Fix
- Fix
- Fix

### 💼 Other

- Initial commit
- Application and main window classes
- Gcc c++11 flag
- Linux ignores
- Removed unneccessary qualifiers
- Debug flag
- Db support + character repository
- Better ignore file
- About box
- Enabled flag
- Key repository + character foreign key
- Character management dialog stub
- Template repository and entities + keyedit dialog stub
- Generic repostiory passing
- Removed sql debug
- Key manipulation
- Key creation link
- Character page stub
- VS build fix
- Button icons
- Crash fix
- Null foreign key support
- Main icon
- Character model stub
- Character entity fields
- Preferences dialog with network settings
- Evernus license
- Settings default proxy
- Api provider setting
- Eve api interface + fetching character list
- Linux build fix
- Switched to QDateTime for cache management
- Character list cache setter
- Moved api interaction to application class
- Active tasks dialog
- Api error passing
- Clearing old tasks
- Aggregating subtask errors
- Better error text propagation
- Api xml parser
- Character list parser
- Minor code adjustements
- Character repo bindings
- Cache db
- Msgbox on initialization exceptions
- Character list caching
- Missing db for model
- Import character preferences
- Additional character fields
- Disabling characters without keys
- More info on error
- App version setting
- Typo
- Additional character properties
- Character sheet import
- Splash screen
- Linux build fixes
- Fetching characters subtasks
- Missing cache_until deserialization
- Character model
- Refreshing/deleting characters
- Menu bar character combo
- Enabled characters change signal
- Character widget stub
- Showing basic character info
- Switched to integer data type for isk
- Small gui tweaks
- Standing edits
- Skill edits
- Windows build fix
- Generic character portrait
- Portrait downloading
- Ccp license
- Eve db
- Missing reference
- Market group
- Market group repository
- Eve type
- Eve type repository
- Missing default values
- Item
- Small initialization fixes
- Cached item
- Cached item repository
- Asset list
- Cached asset list
- Cached asset list repository
- Windows build fix
- Removed margins for menu bar widget
- Status bar wallet info
- Asset key permission
- Import all button
- Button with timer widget
- Import character button
- Asset cache refresh
- Loading and saving asset cache
- Linux build fix
- Path preferences
- Price preferences
- Margin tool stub
- Fetching new id fix
- Fetching assets + asset widget stub
- Margin tool gui
- Merge
- Always on top windows fix
- Margin tool directory watcher
- Updating margin tool on new logs
- Linux build fix
- Closing margin tool with main window
- Margin and markup calculation
- Cached conquerable stations
- Returning real error on prepare
- Minor optimization
- Fetching conquerable stations
- Tax rates in margin tool
- Misleading error fix
- Api timer in asset widget
- Hour view
- Stopped timer text fix
- Common tab widgets base
- Import assets settings
- Batch asset import
- Assets changed signal
- Removed unneded columns
- Changed isk type to double
- Updating character page on char refresh
- Calculating additional data + watching file modifications
- Multiple same request guards
- Responsiveness improvements
- Margin coloring
- Removed unnecessary parameter
- Gui fixes
- Import timer refresh fix
- Eve db connection
- Name provider
- Removed duplicate price data
- Settings item name
- Asset list repository
- Storing assets in main db
- Asset model stub
- Windows build fix
- R/w open file trick on market logs
- My orders filter
- Volume labels
- Removed unused sets
- Volume/movement calculation
- Minimizing main window on margin tool show
- Restored old waiting for eve to finish dumping market data on linux
- Autocopy
- Sample data tables
- Conquerable stations repository
- Clearing conq stations db on refresh
- Asset model
- Asset list swap fix
- Missing settings character id
- Missing settings parent
- Missing default columns
- Asset view data
- Asset info label
- Proper order range checking
- Parsed log removal
- Sell order range fix
- Wallet snapshots
- Refresh character from cache fix
- Clang asset model fix
- Item price repository
- Asset sell price info
- New key link redundancy
- Fetching item prices from eve market data
- Windows build fix
- Additional/valid asset model numerical data
- Switched label order
- Price delta feature
- Switch to gpl
- QCustomPlot
- Cmake adjustements for qcustomplot
- Import prices from web icon
- Storing asset total value
- Resetting all labels on missing data
- Changed db datetime type
- Plot widget with date filters
- Statistics widget
- Removed unneeded delete
- Error message box
- Ref type repository
- Caching ref types
- Batch store in base repository
- Indexes on cache_until
- Clang lambda copy bug fix
- Missing file open
- Windows build fix
- Auto id batch store fix
- Proper id column removal
- Wallet journal repository
- Wallet journal entry ignore option
- Wallet journal widget stub
- Enabled flag resetting on char update fix
- Linux build fix
- New key mask
- Wallet journal api interface
- Cached wallet journal repository
- Ignoring 0 ref type
- Cached wallet journal repository proper base
- Wallet journal
- Wallet journal cache functionality
- Fetching character wallet journal
- Windows build fix
- Dynamic max batch row insert limit
- New icons
- Importing prices from market logs
- Avoiding double delete
- Deleting old wallet journal entries
- Crash fix
- Informative labels
- Storing snapshots with journal
- Interactions and style improvements
- Dynamic axis rescaling
- X label visibility control
- Wallet journal model stub
- Move assignment optimization
- Missing batch size limit
- Missing setting cache data
- Proper unneeded cache item deletion
- Wallet journal widget data fetching
- Switched caching mechanism to network cache
- Importing wallet journal in "import all"
- Cache timer repository
- Storing cache timer in main db
- Windows - choosing between import methods
- //bugreports.qt-project.org/browse/QTBUG-40153 workaround
- Eve cache path settings
- Price import from cache stub
- Determining eve cache paths
- Max price age setting
- Setting correct time spec
- Price age visualization in model
- Removed unneeded moves
- Removed unneeded string processing
- Removed magic constants
- Removed unneeded include
- Wallet journal model showing data
- Wallet journal filtering
- Proper wallet journal sorting
- Wallet transactions in default key mask
- Wallet transaction repository
- Minimum character combo width
- Wallet transactions widget stub
- Moving-constructing object in xml receiver
- Missing constructors and operators
- Fetching wallet transactions
- Windows build fix
- Eve cache parser stub
- Cache node base
- Reading cache stream start
- Removed cache scrapping
- Proper asset model sorting
- Showing wallet transactions
- Leaf filter proxy model
- Asset list filtering
- Wallet journal plot
- Removed cache path settings
- Locale improvements
- Windows build fix
- Ignoring ingored entries
- Merge
- Wallet transaction graph
- Removed plot vertical grid
- Controlling import log wait timer
- Market orders in predefined key
- Fetching market orders
- Copy and move for Entity (big thanks to VS2013 screwed template instantiation)
- Market order repository
- Market order repo fixes
- Fetching market orders
- Main tabs lazy loading
- Signal rename
- Windows taskbar progress
- Slot rename
- Additional info on long imports
- Item costs + widget stub
- Removed unneeded signal
- Using custom price in margin tool
- Reduced initial range filter
- Market order value snapshots
- Saving market order value snapshot
- Multithreaded statistics widget update
- Market orders in statistics (no interpolation yet)
- Optimized statistics calculation
- Merging all graphs
- Switched default statistics range to 7 days
- Item cost model
- Balance summing optimization
- Updating prices by margin tool
- Saving asset value on margin tool hide
- Order info in character tab
- Item cost edit dialog stub
- Adding item costs to db
- Cost filtering
- Crash fix
- Item cost edit/remove
- Merging market order state
- Settings market order cache timer
- Saving market order delta
- Market order models and views
- Market order provider
- Missing translations
- Proper market order first seen setting
- Additional index
- Sell model data
- Market order view info row
- Resizable model views
- Transaction list in order widget
- Buy price providing
- Custom tree view
- Market order icons and tooltips
- Active tasks throbber
- Market order info widget stub
- Better 'about' dialog
- New ignore
- Increased import limit thanks to

https://bugreports.qt-project.org/browse/QTBUG-3024?focusedCommentId=249789&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-249789
- Better throbber
- Update timers
- Proper last seen calculation
- Finished order info widget
- Proper item price import
- Linux build fix
- Market order volume delegate
- Saving margin tool position
- Moved filter to toolbar layout
- No break fix
- Saving/restoring paiter state
- Missing translations
- Indentation fix
- Marker order model grouping
- Warning fixes
- Copy new prices on incomplete data
- Changed sort filter model to leaf
- Changed filter type to wildcard
- Meta group repository
- Showing meta icons in asset tab
- Meta icons in market orders
- Max market order age
- Missing storyline meta icon
- Expired order recognition
- Icon license
- Filtering by order state
- Quit in margin tool
- Remembering last order state filter
- Filtering by order price status
- Market order wildcard filter
- Reversed delta computation
- Ghost grouping items fix
- Range unserialization fix
- Indentation and fixed order info retrieval
- De-magic-numbered volume column
- Market order buy model
- Buy tab
- Default delta for new orders
- Order count fix
- Importing orders from logs
- Unused field removal
- Saving position on quit
- Saving current order tab
- Combined sell & buy tab
- Removed Archived order state
- Portrait context menu
- Import preferences
- Precaching update timers
- Update time warning bar
- Fetching archived orders for character
- Order cache repopulation fix
- Text utils
- Importing prices from orders tab
- Archived state update fix
- Dropped beta version
- Minimize to tray
- Update info list in character widget
- Order history tab
- Removed obsolete function call
- Removed direct sql queries from outside repositories
- Range filter in order history
- Archiving obsolete orders
- Updating timers on individual import
- Updating order count on character sheet
- Removed unused field
- Missing translations
- Switched entity by-value passing to shared ptrs
- Range end filter fixes
- Missing setting delta
- Text filter repository + widget
- Filter widget in tabs
- New splash screen
- Refreshing filter texts in widgets
- Informing order tab about price changes
- Crius db
- Removed type 0
- Removed unnecessary constraint names
- Deferred character cache timer storing (fixed fresh character import)
- Index based order type filter support
- Solar system jumps
- Proper use of prepared statements
- Removed magic constants
- Broke too long lines
- Solar system, region and range in item prices
- ItemPrice -> ExternalOrder rename
- Removed currently unused index
- Importing all external orders from web
- Importing all external orders from logs
- Removed setting external orders via EveDataProvider
- Importing all external orders from margin tool
- 64bit external order id
- Removing own orders from external
- Deferred parsed order store
- Retrieving best sell order
- Deleting obsolete orders after import
- Removed unused method
- Dropped unnecessary index
- Removed whitespace
- Jump map cache
- Solar system id in conquerable stations
- Solar systems
- Missing update after price change
- Retrieving best buy price stub
- Order range check
- Completed returning best buy order
- Translation support in preferences
- Missing Q_OBJECT macros for l10n
- Pl translation
- Initial language select dialog
- Choosing settings default language on startup
- Proper language names
- Qt translations
- Translation fix
- Multiselection copy support
- Proper selection and resizing
- Web price lookup
- Asset list store fix
- Igb service
- Removed unneeded meta object
- Initial language setting fix
- Ts update
- Help link + some icons
- No character chceck
- Updating current order by clicking
- Importing history from mentat
- Plugin loading fix
- Crash fix
- Missing translations
- Translations
- Translations
- Added tag 0.1 for changeset 243197c2caba
- Local time fix
- Missing translation
- Moved tag 0.1 to changeset f19ba0b165d8 (from changeset 243197c2caba)
- Warning fixes
- Removed unneeded library path
- Moved checkbox to button line
- Enable auto close by default
- Advanced statistics

closes #3
- New version check

closes #1
- Translations
- Version bump
- Added tag 0.2-release for changeset 418febfb17fa
- Moved price calculations to separate file
- Displaying - on buy transactions for consistency
- Including skills in profit calculations
- Ability to add custom costs from transactions and margin calculation in orders
- Auto adding custom costs on fulfilled buy order
- Windows compiler bug workaround
- More links in about dialog
- Customizable date/time format
- Margin tool button in order widget
- Removed clear button, since it doesn't apply the filter
- Immediate external order storing after parsing logs
- Custom station price source
- Windows build fix
- QTBUG-40624 workaround
- Disabling buttons after click
- Margin tool help text
- Partial asset value snapshot option
- Version bump
- Translations
- Added tag 0.3-release for changeset a310805fcf93
- Item cost change heavy optimization
- Moved tag 0.3-release to changeset f49089c1d8be (from changeset a310805fcf93)
- Switched to order data source by default
- Word wrapping in preferences label
- Sharing costs between characters
- Auto item cost race condition fix
- Corporation key management
- Corp repositories
- Corporation journal and transactions tab
- Corporation orders tab
- Connected signals
- Corp API support
- Import corporation settings
- Fetching corp wallet journal
- Visual studio compiler bugs workaround
- Crop journal import fix
- Fetching corp transactions
- Rearranged tabs
- Corp timer warnings
- Importing corp orders from logs
- Importing corp orders from the API
- Cache timer duplicate entires fix with migration
- Version bump
- Missing group box
- Translations
- Added tag 0.4-release for changeset 81269659988e
- Corporation orders in igb
- Removed inline editing
- Combined statistics option

closes #7
- Script order processing
- Windows build fix
- Type and location name getters in scripts
- Auto import functionality
- Email notifiactions with auto-import
- Missing password echo mode
- Smtp decryption fix
- Basic http service
- Http service character selection
- Windows build fix
- Market log file name fix

fixes #9
- Corporation id in character and market orders
- Migrating db from 0.4
- Fetching corp orders by corporation id

fixes #8
- Crash fix
- Character/corp orders in http service
- Translations
- Template fixes
- Uncaught exception fix
- Character order fix
- Secondary sorting by overbid/undercut status

closes #10
- Invalid buy model filtering fix
- Corporation id in script order
- Added tag 0.5-release for changeset 5e9271d53f12
- Use IPv4 for http link
- Turn off price status filtering for generic order models
- Eta of order completion

closes #11
- Saving order view header state
- Script order filtering

closes #13
- Backed out changeset: d889408d9114
- Backed out changeset: d13a5f863e9e
- Reading share map
- Rebase fix
- Cache import button
- Parsing more nodes
- Created cache import branch
- Backed out changeset: d889408d9114
- New translations
- Backed out changeset: d13a5f863e9e
- Reading share map
- Marge rebased branch
- Finished basic cache parser
- Switched to QString
- Filling order values from cache
- Icon path typo
- Translations
- Auto-scan feature
- Merge
- Cache import warning
- Button arrow bug workaround
- Plot number format setting
- Merge with default
- Zamknij gałąź cache-import
- Merge with cache-import
- Version bump
- Added tag 1.6-release for changeset 9514eba4cfb4
- More descriptive error message
- Skip character check for corp order logs
- Auto guessing cache path
- Version bump
- Layout fixes
- Added tag 1.7-release for changeset 3e5aaf66a75d
- Changed the custom cache path to point only to the cache folder
- Corp log orders import fix
- Transactions import fix

fixes #19
- Importing character orders into external and dynamic filtering on query
- Refreshing asset model on potential external order change
- Removed obsolete signal
- External order value -> price
- Switched to case sensitive completion
- MarketOrder locationId -> stationId
- Typo fix
- Copy to clipboard buttons
- Hash/uuid based api cache
- Missing Q_OBJECT
- Missing translations
- Build fix
- Version bump
- New data in ExternalOrder
- ExternalOrder locationId -> stationId
- Initial market browser
- Windows build fix
- Removed root decoration
- Populating browser navigator only with available locations
- Disabling api import button by default
- Consistent warning bar for invalid character
- Filtering by regions, systems and stations
- Deviation calculation
- Type id selection fix
- Custom cost in external order view info
- Your order tooltip
- Back/forward support
- Info label
- Favorite item + repo
- Combined updating prices from web
- Item type selection dialog
- Favorite items tab
- Removing favorite items
- Adding to favorites via context menu
- External order filtering
- Location bookmarks
- Order gropuing
- External order buy mode and view
- Setting deviation from context menu
- Buy deviation fix
- Better deviation alignment
- Buttom arrow bug workaround
- Recreating external order table on update
- Translations
- Added tag 1.8-release for changeset 20b0fb20447e
- Stop at end IGB option
- Item history tab
- Version bump
- Api manager return by && + auto max batch size + corpId in journal and transactions
- Adjusting combo size on data change
- Windows build fix
- Removed fk for corp tables
- Margin column in buy orders
- Translations
- Column show/hide
- Automatic price refresh after order import
- Deleting processed cache files
- Added tag 1.9-release for changeset b33a72192f3e
- Moved tag 1.9-release to changeset 8fb1e097d790 (from changeset b33a72192f3e)
- OSX bundling support
- Missing / after BUNDLE_PATH
- Correct macdeploy exec path and add bundle version
- Add icons to osx bundle
- Make install support for Linux
- Thousands separator in plots
- New default plot number format
- All-column order filtering
- Queue close() instead of calling for correctly closing dialog window on osx
- Merged in Neos3452/evernus (pull request #4)

Queue close() instead of calling for correctly closing dialog window on osx
- OS X potential crash fix
- Osx menu roles and shortcuts
- [OSX] Change build type to MinSizeRel

Also corrects Info.plist according to Xcode suggestions.
- Name cache in buy model
- Corp order fix in igb
- Name cache in sell model
- Best order deviation reference fix
- Best margin column
- Price difference column
- Combined corp and char orders
- Settings default values refactoring
- Competing orders in orders tab

closes #22
- Ability to toggle legend

refs #23
- Windows build fix
- Crash fix
- [OSX] Ignore first WindowDeactivate on MarketOrderInfoWidget

For unknown reason focus is being stolen from Order Info Popup right
after window is activated. This is a workaround for this popup
destruction on show.
- Merged in Neos3452/evernus (pull request #6)

[OSX] Ignore first WindowDeactivate on MarketOrderInfoWidget
- Removed corp wallet snapshots

fixes #23
- Limiting to stations

fixes #21
- 64bit corporation id

note: changed type to string in scripts
- Contracts with repository
- Fetching contracts
- Saving contract update timers
- Duplicating own corp contracts in character's
- Basic contract model and view
- Windows build fix
- Hyperion database
- Generic name fetching for orders and contracts
- Assigned contract model and view
- Filtering contracts by status
- Translations
- Version bump
- Contract items with repository
- Fetching contract items
- Windows build fix
- Import contracts option
- Showing contract items
- Button layout bug workaround
- Translations
- Warning fix
- Fetching items for corp contracts
- Ending contract task with empty contract list
- Removed invalid fk
- Windows crash fix
- 32bit warning fixes
- Added tag 1.10-release for changeset 2534a905425d
- [OSX] Change build type from MinSizeRel to Release
- [OSX] Info.plist 1.10 version
- Merged in Neos3452/evernus (pull request #7)

OSX build type change and release version change to 1.10
- Warning fixes
- Removing whitespace for verification codes
- Saving/restoring header state for all views
- Cppcheck fixes
- Not showing external orders for order history
- Favorite item support in IGB
- Translations
- Windows build fix
- Overbid orders in IGB
- Below min margin orders in IGB
- Async batch store
- Copy suggested price in transaction view
- Windows build fix
- Version bump
- Character removal fix
- Contracts in character widget
- Hide contract message if import disabled
- Merge
- Better cmake windows 64bit workaround
- Ignoring user input on task starting
- Unix EOL
- Linux build fix
- Corp wallet snapshot repo + some sanitizing
- Saving corp wallet snapshots
- Corp wallet balance plot
- Combining char and corp journal/trans data
- Api user agent
- Eve-marketdata request partitioning

fixes #25
- Translations
- Suggested price copying in external orders
- Removing orders from context menu
- Market browser quick link in order context menus
- Number formating
- Auto cache import on scan end
- Removed empty import target restriction for log import
- Showing market orders directly in eve via igb
- Windows build fix
- Auto order import without WebSockets, since IGB doesn't support them
- Open in eve by XHR long polling
- Added tag 1.11-release for changeset 1327adb455be
- [OSX] Version 1.11
- Version bump
- Using the correct provider

fixes #29
- Option to use packaged size for ships

closes #30
- Removed unused function
- Crash fix when http client closes the connection before a reply is sent
- Added tag 1.12-release for changeset 03e81c4f7093
- [OSX] Update version to 1.12
- False positive order warning
- Version bump
- Ignore cached data option
- Default market order import source option
- Async task ending
- Corp market order value snapshots
- Saving corp order value snapshots
- Combined corp and char order value plot

fixes #31
- Unique table names

refs #34
- Price differece % column

refs #34
- Auto-copy non-overbid prices option

refs #34
- Igb unique order types

closes #32
- Transaction widget info panel

refs #33
- Customizable market order view info

refs #33
- Ability to specify cutom prices in margin tool
- Total profit in transaction view

refs #33
- Updating generic order info

refs #33
- Clearing filter on script change

refs #35
- Removing filter texts

refs #35
- Restoring main window on margin tool close

refs #35
- Quick date filter

refs #35
- Show/hide tab functionality

refs #35
- Allowing only one instance of the application
- Auto costs from corp orders

refs #35
- Clearing all external orders
- Clearing external orders for type
- Table column menu helper
- Correctly initializing column menu
- Added tag 1.13-release for changeset 8abb002cc00b
- [OSX] Add Xcode files to .hgignore
- Add centralized versioning

Version is now defined in CMakeList.txt and will be updated in Qt and
Info.plist when cmake command is run.
- Include generated version file, not the template.
- Removed unused variable
- Version bump
- Lmeve preferences and tab stub
- Lmeve task entity
- Lmeve taks model stub
- Sync branch
- QtDropbox
- Sync preferences
- Sync dialog stub
- Two-way synchronization
- Hiding token group on proceed
- Compressing sync data
- Skipped vacuum
- False positive order warning
- Separate sync upload and download
- Sync menu action
- Merge with sync-dropbox
- Close sync-dropbox branch
- Merge with sync-dropbox
- Sync translations
- Lmeve task repository
- Lmeve interface and manager stub
- Fetching lmeve tasks
- Lmevetask model
- Windows build fix
- Preventing overwriting newer sync data
- Async data compression + some sync guards
- Guard against cloud data overwriting
- Character select menu
- Layout exception safety
- Potential crash fixes
- Custom cost and margin in tasks
- Fulfillment detection on missing orders
- Buyout in margin tool
- Range inclusion in buy price retrieval
- Using price delta for calculations
- Lmeve profit column
- Saving lmeve station
- Warning fix
- Button text fix
- Change default taskStarted signal emitting to QueuedConnection

On OSX this prevented MenuBar from activating when ActiveTasksDialog
closed.
- Change raw multiline strings into narrow strings

Xcode does not parse multiline strings yet. This will also remove
extra whitespaces.
- Change raw mulitline strings into regular strings

Xcode does not parse mulitline strings.
- Added tag 1.14-release for changeset e3a0c495fa9e
- Typo fix
- Added tag 1.15-release for changeset 59dbe7cde293
- Station view
- Custom price station for assets
- Moved sell station to the right for consistency
- Hack to hide preview widget
- Possible crash fix
- Restoring extenal order view header state on model change
- Version bump
- Remembering standings on import
- Replacing duplicated item ids
- Updater refactoring
- Removed ISK from margin tool edits
- Windows build fix
- Cosistent margin tool edit values
- Added tag 1.16-release for changeset b190bb4d21c8
- Removed tag 1.16-release
- Trade advisor link in igb
- Changed vert size policy for OS X
- Added tag 1-16-release for changeset 23854c7f1d5b
- Crash fix
- Moved tag 1-16-release to changeset f36a1604397f (from changeset 23854c7f1d5b)
- Version bump
- Updating db mod time to avert false sync download
- Uploader settings
- Market uploader thread
- Market uplaoder status in status bar
- Close market-uploader branch
- Merge with market-uploader
- Oceanus db
- Unified market uploader status labels
- Translations
- Endpoint url change
- Added tag 1-17-release for changeset 526716a462cf
- [OSX] Display scroll buttons for main tab widget

Eliding many tabs in main window resulted in unreadable - single
letter tabs. This will turn off eliding for OSX port and display
scroll buttons instead.
- Queue importer signals to EvernusApplication

On OSX direct signal emitting would sometimes steal focus from main
window.
- Queue importer signals to EvernusApplication

On OSX direct signal emitting would sometimes steal focus from main
window.
- Translation update
- Version bump
- Restoring margin tool geometry after flag change
- Added tag 1-18-release for changeset 501da02f616f
- Better size policy
- Version bump
- Switch to escrow for buy order value calculation
- Injecting script api for order filters
- Phoebe db
- Added tag 1-19-release for changeset 6d5d741a9aa2
- Version bump
- Option to omit currency symbol
- Region column in market browser
- Eve api journla import bug

fixes #39
- Allowing for multiple retries

refs #39
- Added tag 1-20-release for changeset 977476264665
- Qt 5.4 build fix
- Always on top for main window
- Warning for qxmlpatterns for < 5.4
- Keeping view expanded after data update

fixes #41
- Rhea sde
- Crest api stub
- Replaced eve-marketdata.com importer with crest
- Crest manager response handling
- Crest authentication
- Fetching orders
- Saving crest cookies
- Windows build fix
- Sell order fetch fix
- Inserting fetched orders
- Translations
- Qt 5.4 build fix
- Seprate crest settings
- Remembering refresh token
- Using byte array to store token
- Guessing solar system id by station
- Close crest branch
- Merge with crest
- Case insensitive order sorting
- External order import task progress reporting
- Fast price copy
- Version bump
- Windows build fix
- FPC help
- Scrolling to current item
- OSX build fixes

Carbon added as dependant framework.
- Do not register empty shortcut in FPCController

Registering empty shortcut in OSX caused 'a' to be stollen from OS.
- [OSX] Fix plots painting over status bar

Why does it paint over status bar is unknown. However repainting
status bar on tab change prevents artifacts.
- Added tag 1.21-release for changeset 5b08a5b3dba5
- Version bump
- Checking old qt version for xml patterns
- Trans update
- Fpc sound
- Merge OSX changes to 1.21
- Simple readme
- Removed local .file
- Trans update
- Merge
- Merge
- Merged krojew/evernus into default
- Removed EMDR uploader
- Ability to set eve destination from asset view
- Ability to set crest via command line
- Moved crest token fetching to CRESTExternalOrderImporter
- Windows build fix
- Empty asset list fetching fix
- Lang update
- Merge
- Lang update
- Async crest order fetching
- Aggregating crest errors
- Full async order fetching to fix stack overflow
- Added tag 1.22-release for changeset 3aa9cf8df86d
- Version bump
- Combining statistics fix
- Windows build fix
- Debug build fix
- Removing obsolete orders by region
- Removed cache importer
- 1.23 migration
- EvernusApplication.cpp edited online with Bitbucket, fixed small spelling error.
- Merged in labarna/evernus/labarna/evernusapplicationcpp-edited-online-with-1419262473986 (pull request #15)

EvernusApplication.cpp edited online with Bitbucket, fixed small spelling error.
- Moved crest auth to CRESTManager
- Initial market analysis widget stub
- Vacummed sde
- Showing task dialog without activating
- Pending region requests
- Missing signal
- Saving imported orders for analysis
- Warning fix
- Refreshing characters on key removal
- Probable contract item import fix
- Overwriting obsolete orders and not saving new option
- Disabling close button when tasks are active
- Ending task
- Restored market groups
- Missing noexcept
- Type tree model
- Switched to type tree from flat list
- Ending task when all items are ignored
- Saving all orders

fixes #44
- Key fix
- Fetching market history
- Saving imported data
- Unneeded sort
- Taking volume into order total prices
- Requesting auth on expired token in-between requests

fixes #49
- Consistent min suffix
- Deleting old market orders
- Cert error handling
- Returning 0 for BPC prices
- Ability to force utc date/time

closes #47
- Showing basic market trade advisory
- Removed ignore existing
- Filtering by solar systems
- Volume/margin filtering
- New QCustomPlot
- Detailed item market analysis
- Remembering last region
- RSI chart
- MACD chart
- Color fix
- Bollinger bands
- Color fix
- Limit for future dates
- Preventing positioning outside of every screen

fixes #53
- Getting ids directly from JSON data
- Remembering previous character

fixes #52
- Function name fix
- Translations
- Proteus sde
- Added tag 1.23-release for changeset 1de229b55961
- Change OSX target deployment to 10.7 from latest(10.10)
- Option to refresh data for all characters at once
- Clearing finished items only on dialog close
- Reversed sorting for unknown eta

fixes #54
- Contract import freeze fix
- Combining asset lists
- Combining wallet journal
- Combining wallet transactions
- Version bump
- Translations
- Added tag 1.24-release for changeset 9474580d8a7e
- Moved filtering onto the DB
- Build fix
- Ignoring existing orders
- Context menu for types
- Restricting margin tool pos to desktop
- Turned off alt log import by default
- Crest login window modality bug workaround
- Preventing accidental close
- Minimize by margin tool option

closes #56
- Better info text
- Handling new preferences for margin tool
- Some logs
- Showing url for crest auth
- Using external browser for authentication
- Switched to new crest app
- Including amount in value computation

fixes #57
- Remembering filter settings
- Saving selected regions
- Saving selected types
- Optimized crest order fetching
- Limitng price copy to custom cost

closes #59
- Human-friendly error message for corp keys
- Translations
- Version bump
- Added tag 1.25-release for changeset 8bbe429a0dcd
- Aligned prices right
- Aligned amounts in wallet
- Taking quantity into totals computation

fixes #60
- Missing auth scheme
- Another missing auth scheme
- Marking old fulfilled orders for auto costs
- Removed duplicated code
- Tiamat sde
- Version bump
- Added tag 1.26-release for changeset e509bae32e2f
- Version bump
- Notes in orders
- Applying initial filter

fixes #66
- Ubuntu clang crash fix
- Removed market scan
- Market analysis trend line
- Windows build fix
- Discarding redundant price delta on custom order source

fixes #67
- Journal and transaction summaries
- Workaround for invalid issue date in market logs

fixes #70
- Added tag 1.27-release for changeset b7e8458fa141
- Multiple crest interfaces
- Merge
- Capped max crest threads due to unofficial request limit
- Sqrtf fix
- Dead store
- Discarding empty price data

fixes #72
- Pre-creating character import tasks

fixes #50
- Crest thread count info
- Translations
- Scylla sde
- Adding item costs for custom characters
- Disabling costactions for no selection
- Version bump
- Added tag 1.28-release for changeset 0728e692a33c
- Normalized diff order property
- Missing script properties
- More qt translations
- Initial German translation
- Removed trade advisor link
- Restoring proxy settings on restart

fixes #75
- De trans
- Trans update
- Typo fixes
- Removed uniform initialization because of tr()
- Trans update
- Version bump
- Added tag 1.29-release for changeset cd7377ee7d3e
- Increased max limit
- Moved tag 1.29-release to changeset 39ad7e98420b (from changeset cd7377ee7d3e)
- Removed .qm from repo
- Reply timeout for crest
- Added timeout to xml api
- Copying data from market analysis
- Workaround for too many timers on Windows
- Windows buld fix
- Wrong clock type fix
- Decreased default max crest threads
- Contract info only looks at in progress and outstanding contracts
- Removed unneeded default param
- Mosaic sde
- Version bump
- Increased timeout
- Clearing obsolete types
- Added tag 1.30-release for changeset 78970332272e
- Max reply time setting
- Drastically increased crest timeout, because crest is bad
- Version bump
- Added tag 1.31-release for changeset 618bb4f98aab
- Max reply time setting max boundary
- Copying prices on checkbox toggle
- Not counting top-level BPCs for asset value calculation

fixes #80
- Consistent overbid detection

fixes #81
- Best margin column in sell model + margin color helper refactoring

closes #85
- Missing translations

refs #85
- Add popup for ignoring ssl error for API request

CREST currently handles ssl error with QMessageBox. This will make
regular APIi request ssl errors handling similar to CREST.
- Merged in Neos3452/evernus (pull request #19)

Add popup for ignoring ssl error for API request
- Removed unused signal/slot
- Option to ignore ssl errors
- Bundle mozilla CA certificates with OSX

Newer versions of openssl cannot access system keychain. Why does it
use other then system openssl, I have no idea. For now lets use
mozilla's CA file.

Also added required headers for compiling with QT 5.5.
- Revert obsolete markup from lang

Probably leftover from qt 5.5 compilation.
- Flagging only active orders as overbid

fixes #87
- Carnyx sde
- 1.32 migration
- Version bump
- Added tag 1.33-release for changeset fe14c4cb4035
- Added tag 1.32-release for changeset fe14c4cb4035
- Removed tag 1.33-release
- Removed invalid types
- Moved tag 1.32-release to changeset f339ca0653bb (from changeset fe14c4cb4035)
- Sorting by volume
- Margin tool minimize button
- Workaround for ccp broken date export

fixes #93
- Version bump
- Aegis sde (1.1.1)
- Clearing market analysis types on every update
- Not showing price delta on missing price data
- Scal
- Added tag 1.33-release for changeset 707d9aa7b4a6
- Removed tag 1.33-release
- Updating via maintenance tool
- Vs 2015 ignore
- Added flags for visual studio build

Mulitprocessor compilation
Requirement for (VS == 2013 && CTP_Nov2013) || VS > 2013
- Line endings normalization

<Comment full of rage>
- Proper VS2013 compiler string
- Hack for vs 2015 template calls in lambda error
- Moved header to the right place
- Ts update
- VS2013 build break
- Call startDetached(maintenancetool) with empty program arguments list

Because Program\ Files is a great name for a system path.
- Pedantic name change
- Added tag 1.33-release for changeset 1fd487a7bfa4
- Char/corp tab switch in menu
- Build fix
- Galatea sde
- Restored missing translations
- Translations
- Price filters
- Better flow layout with vertical centering
- Removed debug stuff
- Database versioning
- Removed unneeded \t

fixes #104
- Moved fetching endpoints to crest manager
- Version bump
- Vs2013 fixes
- Added tag 1.34-release for changeset 55cdcba2c9bf
- Compilation error fix
- Moved tag 1.34-release to changeset e289b006b07d (from changeset 55cdcba2c9bf)
- Details tooltip

refs #110
- Retrying to fetch crest endpoints
- Settings stat plot colors

closes #96
- Inter-region analysis stub
- Showing inter-region trade with region filtering
- Volume filtering
- Margin filtering
- Crash fix
- Settings endpoints on crest interface reset
- Discarding bogus orders
- Translations
- Saving region filter
- Displaying progress while calculating
- Separated percentile calculation
- Custom stations
- Updating stations on initialization
- Region sell order price checking
- VS 2013 build fix
- De translations
- Scal
- Crash fix on edit after switching characters
- Removing groups with no types
- Reduced signal frequency
- Vanguard sde
- Version bump
- Added tag 1.35-release for changeset 86a949406074
- Option to use skills for difference calculation

closes #115
- Corp wallet division select

closes #111
- Wallet division in transactions

refs #111
- Price delta randomization

refs #118
- Crop asset support

refs #116
- Alternating sample row colors in margin tool
- Extracted a caching asset provider
- Removed hardcoded libc++ dependency
- Moved stat proefrences to their section
- Separate statistics settings file
- Switched to type safe connections
- Manual value snapshots

closes #119
- Unclogged event loops
- Made a constant more constant
- Moved data fetching to another thread
- Corp assets in statistics
- VS 2013 build fixes
- Solar systems in sde
- Rate limiter
- Crest rate limiter
- Removed authed crest support
- Qt >= 5.4 dependency
- Added timeout to xml api
- Workaround for too many timers on Windows
- Translations
- Windows buld fix
- Wrong clock type fix
- Merge with public-crest-switch
- Translations
- Parallax sde
- Regression fix
- Removed crest thread count setting
- Version bump
- Trans
- Windows build fix
- Removed unused include
- Single timer for pending requests
- Added tag 1.36-release for changeset 9dcd7ea5e1d6
- [OSX] Generate dmg during cmake install phase
- Non-modal active tasks dialog
- Market analysis task aggregation
- Blocking parallel same imports
- Vs 2013 build fix
- Frostline sde
- Translations
- Version bump
- Added tag 1.37-release for changeset 49872be8c365
- January sde
- Custom column data delimiter

closes #138
- Correct Info.plist path for out of source builds
- Use high resolution portrait when DPR >= 2
- MarginToolDialog UI improvements
- Remove unnecessary debug message
- Remove trailing whitespace
- Added tag latest-release for changeset 49872be8c365
- Proxy web importer for multiple web importers
- Eve-central importer stub + preference for change
- Set default web importer to crest
- Eve-central external order importer
- Extracted eve central fetching to spearate class
- Eve central order fetching for market analysis
- February sde
- Backed out changeset: 931fe8af9f01

closes #135
- Changing notes for multiple orders

closes #130
- Added tag 1.38-release for changeset 7b0f2a17b068
- Moved tag latest-release to changeset 7b0f2a17b068 (from changeset 49872be8c365)
- Version bump
- Moved tag latest-release to changeset d36e86d8f466 (from changeset 7b0f2a17b068)
- Moved tag 1.38-release to changeset d36e86d8f466 (from changeset 7b0f2a17b068)
- Render qcustomplot in HDPI using DPR

Pixmap size is multiplied by device pixel ratio to draw in high
resolution.
- Missing initalization
- Added tag 1.39-release for changeset d18193cddb3b
- Moved tag latest-release to changeset d18193cddb3b (from changeset d36e86d8f466)
- Version bump
- Moved tag 1.39-release to changeset 3b22d245cc74 (from changeset d18193cddb3b)
- Moved tag latest-release to changeset 3b22d245cc74 (from changeset d18193cddb3b)
- Respect eve corp/faction standing limits [-10.0,10.0]
- Moved tag 1.39-release to changeset 4cae6e571703 (from changeset 3b22d245cc74)
- Moved tag latest-release to changeset 4cae6e571703 (from changeset 3b22d245cc74)
- Moved crest error gui out of crest manager

fixes #147
- Applying dt format to graphs

fixes #146
- Merge
- Yc-118-3 sde
- Version bump
- Moved tag latest-release to changeset 04871b48f8a3 (from changeset 4cae6e571703)
- Added tag 1.40-release for changeset 04871b48f8a3
- "don't show again" in crest error
- Moved tag latest-release to changeset d1b62d4dad7f (from changeset 04871b48f8a3)
- Moved tag 1.40-release to changeset d1b62d4dad7f (from changeset 04871b48f8a3)
- Separated importer generic error from import error

fixes #148
- Translations
- Improve database write performence by turning off disk synchronization
- Separated order models from single character id
- Combining orders for all characters
- Merge
- Moved sync pragma to one central place
- Endline fixes
- Low volume warning
- Mutiple favorite item adding
- Merge
- Volume flagging
- Switched computations to boost::accumulators
- Expand/collapse all
- Alphabetic default sort order
- Vs 2015 dependency
- Sample data toggle button
- Removed obsolete bug workaround
- Ignoring min volume
- Switched crest address
- Version bump
- New fee calculations + character custom fee
- Trans
- Unified market order endpoint
- Merge with crest-unified-market-orders
- % sign for brokers fee
- Vs ignore
- Switched to JSEngine
- Added tag 1.41-release for changeset e4a0f3957d33
- Moved tag latest-release to changeset e4a0f3957d33 (from changeset d1b62d4dad7f)
- Additional error handling in case finished() is not emitted

https://bugreports.qt.io/browse/QTBUG-53535
- Backed out changeset: cd13e86138eb
- Typo fix

fixes #155
- 20160531 sde
- Bumped crest version
- Getting history endpoint from region data

fixes #166
- Trans
- Whole market externak importer
- Getting url from endpoint map
- Auto importer selection
- Translations
- Crash fix
- Better auto importer choosing
- Removed per-order filtering
- Fetching next page before processing the results
- Serial fetching penlty factor
- Filtering resulting orders
- Whole market import in market analysis
- Msvc generic lambda bug fix
- Marge with crest-whole-market-endpoint
- Version bump
- Moved tag latest-release to changeset 633dc046a08f (from changeset e4a0f3957d33)
- Added tag 1.42-release for changeset 633dc046a08f
- Duplicate line

fixes #168
- Forcing version and disabling db update
- Double avg volume

closes #169
- Adjusting item costs

closes #154
- Out-of-bounds crash fix
- Correct quantity in transaction plots

fixes #175
- Limiting price to total costs

closes #174
- Removed code repetition
- 20160809 sde
- Multiple item histories

closes #159
- Translations
- Version bump
- Added tag 1.43-release for changeset 7a76d1b94fbb
- Moved tag latest-release to changeset 7a76d1b94fbb (from changeset 633dc046a08f)
- Opening market details via crest
- Per-character access tokens
- High DPI mode
- Adjustable market analysis views

closes #122
- Forcing c++14 standard

refs #187
- Show in eve in assets

refs #185
- Setting destination via crest
- Removed IGB service
- Version bump
- Added tag 1.44-release for changeset 7a48b778c044
- Moved tag latest-release to changeset 7a48b778c044 (from changeset 7a76d1b94fbb)
- Bump OSX min requirement to 10.8

error: std::future was added in osx 10.8
- Remove mozilla cert file in favor of native osx ssl backend

From Qt 5.6 onwards Qt uses native MacOS backend for secure communication so
it is no longer required to bundle certificates for outdated openssl.
But there is a problem with LibQxt which relays on QT_NO_OPENSSL to disable
secure connection options even though they work with MacOS backend. We trick
LibQxt by defining QT_OPENSSL which makes qtconfig.h undefine QT_NO_OPENSSL.
- Build fix

fixes #198
- Version bump
- Separate buy and sell broker fee

closes #160
- Show in eve for transactions

closes #185
- Show in eve on fpc
- Translations
- Ignoring types from corp orders

fixes #186
- Setting per-order custom station

closes #180
- 20161114 sde
- Remembering custom station on import
- Added tag 1.45-release for changeset 0d8fb56d5e55
- Moved tag latest-release to changeset 0d8fb56d5e55 (from changeset 7a48b778c044)
- Deleting only first row of selection

fixes #201
- Fpc showing selected item in eve, instead of current

fixes #202
- Version bump
- Added tag 1.46-release for changeset 8687c7d4ea59
- Moved tag latest-release to changeset 8687c7d4ea59 (from changeset 0d8fb56d5e55)
- Silenced a warning
- Support for assets lists without items

fixes #208
- CMakeLists.txt edited online with Bitbucket
- Scal
- 20161213 sde
- Opening in eve for asset owner

closes #214
- Exposing custom cost to scripts

closes #211
- Using custom asset location for value computations

closes #162
- Choosing price type for market ana;ysis

closes #216
- Version bump
- Color tag support

closes #183
- Fetching citadels
- Returning citadel names
- 20160112 sde
- Added tag 1.47-release for changeset 80facb2ed91d
- Moved tag latest-release to changeset 80facb2ed91d (from changeset 8687c7d4ea59)
- Validating cost
- Clang build fix
- Switch to automoc and autorcc
- Switched crest to esi
- Default custom station

closes #227
- Returning more data about citadels

fixes #182
- Unified location id type
- Custom range threshold

closes #222
- Custom fpc data support

closes #217
- Initial dropbox 2 api support
- Handling file errors
- Using local web server if possible
- Backwards fpc support
- 201603 sde
- Version bump
- Added tag 1.48-release for changeset f3fc3d8dbd19
- Moved tag latest-release to changeset f3fc3d8dbd19 (from changeset 80facb2ed91d)
- Fetching orders from citadels
- Proper signal handling
- Missing subject
- Another missing subject
- Not retrieving data from invalid record
- Missing region citadel clear
- Not fetching invalid record
- Fetching citadel orders from importer
- Propagating paginated errors
- Splitting multiline errors
- Suppressing auth errors
- Some help text
- Filtering citadel orders
- Typo
- Esi request retries
- Help info when importing from citadels
- Replaced qcustomplot with qtcharts for statistics
- Replaced qcustomplot with qtcharts for item history
- Removed label hiding, sice charts weren't resizing
- Zooming support in charts
- Info about time
- Properly attaching axes
- Toggle info
- Replaced qcustomplot with qtcharts for aggr item details
- Removed qcustomplot
- Backed out changeset: 78bea14e50c7
- Backed out changeset: 197d79800f9c
- Backed out changeset: ca3e103ce46c
- Backed out 605240832e6a
- Backed out changeset: 55898099ac09
- Backed out changeset: ed451d78b368
- Backed out changeset: 7a376c8c2b7b
- Brought back qcustomplot1
- Added type id to citadels
- Not overwriting db version with older one
- Not fetching orders from Astrahus
- Disable double popup
- Some price type info
- Using different source price type by default
- 119.5 sde
- Added tag 1.50-release for changeset 1afb31c57ffc
- Moved tag latest-release to changeset 1afb31c57ffc (from changeset f3fc3d8dbd19)
- Ignoring updater errors on up-to-date tables
- Moved tag latest-release to changeset 7ff797e5f405 (from changeset 1afb31c57ffc)
- Moved tag 1.50-release to changeset 7ff797e5f405 (from changeset 1afb31c57ffc)
- 64bit location ids for external orders
- Added tag 1.51-release for changeset e1df6b5a2fe5
- Moved tag latest-release to changeset e1df6b5a2fe5 (from changeset 7ff797e5f405)
- Version bump
- Moved tag latest-release to changeset 212fdf681385 (from changeset e1df6b5a2fe5)
- Moved tag 1.51-release to changeset 212fdf681385 (from changeset e1df6b5a2fe5)
- Not processing the same request multiple times
- Limitng citadel requests per region
- Version bump
- Added tag 1.52-release for changeset 1bd2d2be0b52
- Moved tag latest-release to changeset 1bd2d2be0b52 (from changeset 212fdf681385)
- Custom cost for items
- Version bump
- Properly handling range threshold for station-only buy orders

fixes #237
- Merged krojew/evernus into default
- Moved to proper version
- Adjusting totals when changing ignore status

fixes #238
- Remembering window size
- Showing custom value in asset view
- Changed dirs to match qt deployment
- Correctly diplaying custom value
- Saving/loading custom value
- Using correct station for price status
- Not searching through citadel cache when not needed
- Showing feedback when location name is not found

refs #240
- Setting custom value
- Retries in xml api
- Extracted basic statistics widget
- Extracted advanced statistics widget
- Extracted region analysis tab
- Processing events while making requests
- Removed unused symbols
- Extracted inter region analysis tab
- Details icon + tooltip
- Extracted filter widget
- Extracted graph
- Inter-region market details
- Enabled dropbox debug
- Distinction between limit and market orders for taxes
- Basic importing analysis (no data yet)
- Avg volume should include gaps
- Showing importing volumes and src/dst prices
- Import price support
- Margin column
- Moved columns
- Calculating prefered margin when no sell orders exist
- Properly resizing sections
- Price difference and projected profit
- Clearing old data on import
- Properly showing legend after creation
- Double type compare widget
- Removed copy-pasted price type
- Missing event processing during history import
- Text fixes
- Some tooltips
- Aggregated asset view
- Showing release notes
- Reprocessing table
- Reprocessing skills
- Version bump and updating to 2.0
- Reprocessing skill edits
- Parsing reprocessing skills
- Extracted waiting widget
- Ore reprocessing tab stub
- Extracted common behavior for analysis widgets
- InvGroups table
- Getting reprocessing map skeleton
- Basic arbitrage filtering
- Missing suffix
- Extracted region combo
- Removed history dependency
- Resizing header to contents
- Setting price types and showing in eve
- Extracted station select button
- Region/station filtering
- Passing data to the model
- Finding opportinities skeleton
- Removed mutex since only one thread touches orders for given type
- Calculating sell volume
- Removed magic constants
- Finding opportinities for sell->buy orders
- Not freezing the ui
- Margin
- Better info
- Volume and limit order support
- Context menu support
- Merge arbitrage
- Removed stuff from unsupported qt version
- Refreshing character if character data changed in char widget
- Scrapmetal arbitrage as a copy o the ore one
- Calculating arbitrage for scrapmetal
- Extracted common reprocessing widget part
- Merge scrapmetal-arbitrage
- Not recalculating everything on price type change
- Warning fix
- Filling item list on creation
- Direct slot connection
- Paralellized order/history parsing
- Backed out changeset: 6759749c922b
- Some logging
- Another attempt at speeding up order parsing
- Multi-interface esi manager
- Interface count lower bound
- Initial reports widget
- Updated qt requirement
- Best items report
- Filtering by date
- Profit per m3 column
- Station report
- Hisec filtering
- Market order performance view
- Linux build fix
- Sorting by turnover
- Warning fix
- Help tooltips
- Disabling email notification if some conf is missing

closes #245
- Preemptive change to unix line endings
- Trans update
- Region-type preset repo

refs #228
- Saving/loading presets

closes #228
- Suggesting names to save
- Logging to file
- Warning fix
- Parsing esi error
- Properly ignoring 403
- Unified esi error messages
- Typo
- 119.6 sde
- Added tag 2.0-release for changeset c1352a647ee4
- Moved tag latest-release to changeset c1352a647ee4 (from changeset 1bd2d2be0b52)
- Moved tag latest-release to changeset d0d2d7b986b3 (from changeset c1352a647ee4)
- Moved tag 2.0-release to changeset d0d2d7b986b3 (from changeset c1352a647ee4)
- Proper row count for table models
- Rewrite QMacPasteboardMimeUnicodeText to default to utf8
- Macd tooltips
- Choosing between volume and order count

closes #254
- Order count in region analysis
- Order count in inter-region analysis
- Order count in importing analysis
- Custom default analysis period

closes #253
- Deleting multiple orders

closes #250
- Typos
- Some help texts
- Getting proper station id for import

fixes #261
- Importing explicit order data

closes #259
- Giving time to hide region dlg
- Properly increasing counter on new imports
- Increating order counter on import start
- Deduplicating requests
- Using only sell orders for source
- Better log format
- Correctly handling portion size
- Renamed inlined files
- Version bump
- Added tag 2.1-release for changeset 7457b7b0fa5e
- Moved tag latest-release to changeset 7457b7b0fa5e (from changeset d0d2d7b986b3)
- Updating db version to current one instead of saved one
- Small conditional optimizations
- Collateral in importing

closes #263
- Merge
- Add MacOS guards for copy/paste fix

OSX is used instead of MACOS just in case we would downgrade Qt
- Bump MacOS req to 10.9, same as Qt 5.8
- Osx fixes only on osx
- Not using 0 as a magic number
- Scaling efficiency to 0-1
- Properly computing station tax

fixes #271
- Separated order prices
- Choosing collateral price type

closes #268
- Removed duplicated signal

fixes #248
- Esi threads default tied to core count
- Merged krojew/evernus into default
- Small simplification
- Clearing market analysis models on character change
- Correctly ignoring players skills when selected
- Calculating tax for total reprocessed volume, instead of just sold

refs #271
- Custom station tax

fixes #271
- Packaged size for Capital Sensor Arrays

closes #268
- Packaged sizes for modules

refs #268
- Extracted common stuff for arbitrage models
- Ignored flag for citadels

refs #255
- Version bump
- New forum topic link
- Close button
- Version/os info
- Missing Q_OBJECT macros
- Preserving color tag and notes across updates
- Removed tabs
- Maximize btn
- Synchronous citadel model
- Enabled close button
- Missing custom value for corp items
- Saving custom location radio
- Getting station id from path
- Properly getting custom asset station for value
- Check support
- Info text
- Setting ignored state
- Ignoring market from ignored citadels
- Helper tree view for checkable items
- Selecting empty solar systems
- Saving ignored in batches
- Ru_RU language
- Preserving ignored state after citadel import
- Transactions and string literals
- Option to filter out empty source sell orders

closes #274
- Extracted common parts
- Option to ignore price percentiles

closes #126
- Custom avg period
- Typo
- Literals and transactions
- Tupo
- Updated translations
- Using the latest log file found without timestamp caching

refs #276
- Remembering last log file to not parse again
- Trans update
- Finding last log by iterator

refs #276
- 119.7 sde
- Moved tag latest-release to changeset 2b872e7e4f6d (from changeset 7457b7b0fa5e)
- Added tag 2.2-release for changeset 2b872e7e4f6d
- Adding optional params as they seem to be required
- Some literals
- More literals
- Using data provider for id paths
- And more literals
- Forcing import on auto costs

refs #278
- Descriptive errors when no character present
- Handling missing character name for sso
- Region + station presets

refs #275
- Bumping standard to c++17
- Src/dst combo preset

refs #275
- Switched to non-deprecated enum
- Google breakpad integration for linux
- Enabled breakpad on windows
- Linking with boost imported target
- Adding query to error message
- Default custom station help
- Region present management dialog

refs #275
- Favorite locations in iter-region analysis

refs #275
- Favorite locations in arbitrage

refs #275
- Favorite locations in importing

closes #275
- String literals
- Filtering out unpublished types
- Details in importing

refs #280
- Sql logging
- Updated forum links
- Wrapped storing citadels in transaction

refs #283
- Filling missing data from new citadels

fixes #283
- Median dst volume
- Dst volume MAD
- Disabled vertical dragging in reports
- Enabled vertical dragging again
- Lazy-filling initial data
- Ignoreing orders iported ad fulfilled
- Accuracy info
- Setting focus on item name
- Accepting $ in costs
- Csv import/export
- Trans
- Re-adding ignored column
- Eve import source setting
- Asking for eve import source
- More info text
- Skeleton for fetching assets via esi
- Fetching assets via esi
- Esi character import skeleton
- Importing character via esi + fetching token for many characters
- Version bump
- Importing character market orders via esi
- Saving cache timers from esi
- Clang build fix
- Setting cache timers before update for proper notifications
- Respecting cache timers for esi
- Consistent naming
- Contracts scope
- Consistent char/corp naming
- Type-safe connections
- Consistent char/corp naming
- Esi wallet journal import skeleton
- Esi wallet transaction import (broken on server side)
- Windows build fix
- Properly centering auth window
- Properly sending token requests for muiltiple characters
- Version bump
- Moved core update to the right place
- Thanks
- 119.8 sde
- Order lookups in market analysis

closes #280
- Trans
- Moved tag latest-release to changeset 71eb7fafb2fd (from changeset 2b872e7e4f6d)
- Added tag 2.4-release for changeset 71eb7fafb2fd
- Off by 1 error
- Removed max width for station selection
- Getting manufacturing activity id
- Industry tab skeleton
- Extracted TradeableTypesTreeView
- Manufacturing and type views
- Reacting to types change
- Manufacturing setup stub
- Switched graphics view for qml
- Import button
- Treating esi quantity as xml raw quantity

fixes #292
- Bezier curve qml item
- Version bump
- Main qml view for manufacturing
- Manufacturing type view skeleton
- Manufacturing setup model for qml
- Commented out industry tab creation for release
- Added tag 2.5-release for changeset 14ba1b6d9f32
- Moved tag latest-release to changeset 14ba1b6d9f32 (from changeset 71eb7fafb2fd)
- Brought back industry tab
- Filling type manufacturing info
- Missing manufacturing activity
- Option to turn off breakpad from cmake
- New cmake and turing off breakpad
- Gcc 7 compatibility
- Properly turning off breakpad
- Sign fix
- Creating model from manufacturing setup
- Extracted type location pairs
- Importing data for analysis
- Resizing view to window
- Showing name + iamge in type delegates
- Removed corner radius
- Random string literals
- Getting true manufacturing info (output quantity and type by blueprint id)
- Reorganized files and initial attempt at recursive tree
- Recursively creating material tree
- Aligning items and view move support
- Async children creation
- (incorrectly) creating connections
- More curves
- Removing unnecessary marign
- Int version numbers
- Version bump
- Missing noexcept and inline
- Unified qml version with app version
- Correctly setting connection y
- Little refactoring
- Color support
- Explicit antialiasing
- Animating selection
- Setup controller and selecting all types
- Visual improvements
- Showing quantity produced
- Initial source type combo
- Windows build fix
- View fixes
- Licenses
- Replaced flickable with scroll view
- New sources
- Showing required quantities
- Changing source type for materials
- Not checking screen geom when positioning, since it might not find the correct screen

fixes #94
- Breakpad for osx
- Centering required quantity
- Properly initializing source combo
- Industry activity table
- Showing production times
- Moved manufacturing analysis to a tab
- Runs for output
- Runs input
- Efficiencies in setup
- Switch from plain edit to spinbox
- Setting runs
- Computing effective required quantity
- Removed uneccesary item
- Changing quantity required for all items for given types
- Me/te spin boxes
- Stage one of wallet journal esi format migration
- Finished migrating wallet journal to esi format
- Removed ref types
- Using required materials formula
- Updating me and te
- Extracted required quantity calculation
- Fetching generic names via esi
- Using material efficiency
- Switched to qt controls 2
- Facility/security/rig selection
- Using facility/sec/rig for materials
- Formatting numbers
- Initial time computation
- Proper combo placement
- Manufacturing time implant bonus
- Industry skills table
- Industry skills
- Showing time per single run
- Editing industry skills
- Fetching industry skills from xml api
- Showing details only when bopth hisotries are present

fixes #302
- Importing industry skills
- Final time calculations
- Clang build fix
- Clang build fix
- Showing missing history message
- Showing total manufacturing time
- Saving imported orders for setup
- Initial cost support in model
- Remembering used asset quantities
- Not assigning assets to output and otherwise acquired items
- Filling order data
- Stopped infinite refresh loop
- Always showing info
- Notifying only the first parent of changes
- Properly setting me/te for output/sources
- Disabling me/te for non-manufactured goods
- Omitting currency symbol
- Propagating changes to cost role
- Returning buy cost
- Removing non-manufacturable types
- Filtering orders by src/dst regions
- No job installation costs warning
- Facility tax
- Importing adjusted prices
- Importing cost indices
- Removed unnecessary asset quantities reset
- Removed unnecessary asset quantities reset
- Properly propagating parent role changes
- Computing manufacturing costs
- Missing manufacturing costs for parent
- Moved time formatting to qml
- Showing profit
- Industry manufacturing setup entity
- Saving/loading setup
- Windows build fix
- More intuitive names
- Taking taxes into account
- Properly moving a path instead of creating an element with path
- Separate manufacturing station
- Simplified children creation
- Using timer to force event loop update
- Output creation progress bar
- Processing output size warning
- Showing bill of materials
- Warning fix
- Not showing output and manufactured goods in bom
- Disabling controls when refresh is taking place
- Removed obsolete comment
- Keeping single collection of esi interfaces
- Extracted src/dst selection widget
- Blocked double station change signal

fixes #300
- Enabling controls with no output types
- Setting initial manufacturing station
- Correctly filtering by stations
- Detailed cost tooltip
- Setup summary
- Subtracting cost from profit
- Isk/h summary
- Removed system cost index
- Not inspecting children needlessly
- Showing costs
- Preserving previous setup
- Market volume warning
- Trans
- Switched to boost exceptions
- String literals
- Catching unhandled exception
- Version bump
- Explicit versions for backwards compatibility
- Properly getting profit info
- Extracted char name fetching
- Setup text view
- Importing character blueprints
- Refreshing me/te on tree view
- Using x-pages header for paginated requests
- Blueprint import confirmation
- Separated display from edit roles
- Runs column
- Handling single page paginated responses
- Added tag 2.7-release for changeset c4025a6f0899
- Moved tag latest-release to changeset c4025a6f0899 (from changeset 14ba1b6d9f32)
- Warning fix
- Updating char name in the auth window
- Some literals
- Version bump
- Moved tag latest-release to changeset 72aff63f1fe7 (from changeset c4025a6f0899)
- Added tag 2.8-release for changeset 72aff63f1fe7
- Copying citadels for region to avoid in-the-middle update in event loop

fixes #314
fixes #306
fixes #296
- Helper ignore
- More descriptive exception
- Setup loading error support
- Sized version
- Not updating model whlie initializing
- Version bump
- Added tag 2.9-release for changeset 68820aca8ed2
- Moved tag latest-release to changeset 68820aca8ed2 (from changeset 72aff63f1fe7)
- Wycofano pakiet zmian: e22e55dac326
- Moved tag latest-release to changeset 7879e31e4e25 (from changeset 68820aca8ed2)
- Moved tag 2.9-release to changeset 7879e31e4e25 (from changeset 68820aca8ed2)
- Define EVERNUS_CREATE_DUMPS to 0 if set to OFF

Otherwise Defines.h will define it to 1 (if undefined) and it won't compile,
because cmake will not look for breakpad lib.
- Merge MacOS breakpad cmake changes
- Removed size warning
- Windows build fixes
- Updating db version
- Fetching client name from esi
- Replaced for generic header
- Debug dumping setup
- Backed out changeset: 7879e31e4e25
- Trans
- Some literals
- Clearing cookies

closes #315
- Properly checking all the keys
- Minor refactoring
- Asserting fatching from proper thread
- Fetching generic names from appropriate thread
- Up MacOS requirements to 10.10 to match Qt 5.9
- Deploy qml imports into the .app bundle
- Merged in Neos3452/evernus (pull request #39)

MacOS; qml deploy and up req to 10.10
- Removed duplicated member => ore arbitrage works again
- Trans
- Some more info
- Synchronous flag setting
- Removed qml trans
- Removed ru trans
- Removed dead code
- Total produced quantity

closes #308
- Moved esi interface manager to application
- Ensuring requests are sent from appropriate thread
- Atomic is unneeded, since whole collection needs to be synchronized
- Using proper lock guard
- Version bump
- Moved tag latest-release to changeset 91ed8b060402 (from changeset 7879e31e4e25)
- Added tag 2.12-release for changeset 91ed8b060402
- Verbose error logging
- Switched list selection to check boxes
- CitadelAccessCache for esi interfaces
- Error limiter for 420 status code
- Caching unavailable citadels
- Auto clearing stale data
- Version bump
- Added tag 2.13-release for changeset e0cab639bc9e
- Moved tag latest-release to changeset e0cab639bc9e (from changeset 91ed8b060402)
- Bumped max citadel cache age to 7 days
- Moved tag latest-release to changeset eb04fb9e2c87 (from changeset e0cab639bc9e)
- Moved tag 2.13-release to changeset eb04fb9e2c87 (from changeset e0cab639bc9e)
- Layer cache busting
- Showing blacklisted citadels
- Name change to what's actually done
- Debug log
- Responding only to 403
- More logging
- Refresh access cache button
- Using helpers
- Log rotation
- Set .dmg fs to HFS+

On 10.13 hdiutil default is APFS which is not supported on older systems.
- Merged in Neos3452/evernus (pull request #40)

Set .dmg fs to HFS+
- Refresh icon
- Class name changes
- Handling sso_status value

refs #326
- Removing invalid types from presets
- Version bump
- Icons
- Clearing citadel cache
- Version bump
- Added tag 2.15-release for changeset 8034fe475b06
- Moved tag latest-release to changeset 8034fe475b06 (from changeset eb04fb9e2c87)
- Version bump
- Alpha clone flag
- Updating alpha clone status
- Adding 2% alpha clone costs
- Switched to process to avoid exit on OSX
- Using esi character verification
- Switched eve-central lookup to evemarketer
- Market orders with type filter actually look up all pages
- Better name
- Removed eve-central stuff
- Removed serial fetch penality thanks to x-pages
- Version bump + upgrade
- String literals
- Mining ledger repo
- String literals
- Fetching mining ledger
- Mining ledger widget stub
- Refresh mining ledger stub
- Industry import preferences
- Mining ledger timers
- Missing timer handling
- Added mining ledger tab
- Hooked up ledger import
- Fetching mining ledger
- Using authed call
- Build fix
- Crash fix on auth error
- Some literals
- Missing timer
- Storing ledger
- Saving update timer
- Update to qcustomplot2
- Merged qcustom plot 2
- Gnu c lib compatibility
- Brought back translations
- Warning fix
- Showing mining ledger details
- Using paginated response for assets
- Selecting import source
- Disabling lookup actions
- Proper sorting
- Importing data
- Sell price type selection
- Showing aggregated mined types
- Aggregating by solar systems
- Initial mining ledger 3d visualization
- Better name
- Better default camera
- Better graph placement
- Dropped fractions for quantities
- Using splitter to split content
- Logging esi replies
- Merge
- Warning fix
- Settings prices
- Showing profit in all models
- Changing graph properties
- Zooming to bar
- Remembering last import source
- Moved sso auth window handling to MainWindow
- Name change
- Not resetting auth dialog after a new one was requested
- Closing window on final redirect
- Processing auth codes only by the requestor
- Windows build fix
- Dropped unneeded async from name
- Removed copy-pasted clear
- Clearing saved refresh tokens from preferences
- Version bump
- Properly updating import timer
- Removed unused preset
- Showing profits
- Lifeblood sde
- Added tag 2.18-release for changeset 8ca103814510
- Moved tag latest-release to changeset 8ca103814510 (from changeset 8034fe475b06)
- Missing data vis
- Warning fixes
- Show in eve option

refs #316
- Showing in market browser

refs #316
- Lookup actions

refs #316
- Saving imported prices regardless of error

refs #350
- Version bump
- Added tag 2.19-release for changeset 127199404f25
- Moved tag latest-release to changeset 127199404f25 (from changeset 8ca103814510)
- Warning fixes
- Clearing fetched orders from memory
- Migrated optionals to std
- Importing proxy model
- Filtering in importing analysis
- Passing const dt for each thread
- Using cmake 3.10
- Merge
- Not clearing pending auto costs when no transactions present

refs #345
- Simple ref type transform

closes #360
- Importing corp orders along with character's

refs #342
fixes #149
- Support for out-of-order pages

fixes #363
fixes #342
- Probable crash fix

fixes #364
fixes #362
fixes #352
- 20171205 sde
- Version bump
- Added tag 2.20-release for changeset 19445292c17b
- Moved tag latest-release to changeset 19445292c17b (from changeset 127199404f25)
- Migrated esi to dev
- Typos
- Forcing data refresh

fixes #369
- Version bump
- Added tag 2.21-release for changeset e82252a670ea
- Moved tag latest-release to changeset e82252a670ea (from changeset 19445292c17b)
- Removed key and xml api code
- Fetching conquerable stations from sov structures
- Passing expire time to non-persistent data
- Removed key repo references
- Missing header
- Removed obsolete files
- Fetching char contracts from esi
- Fetching char contract items via esi
- Refreshing all characters
- Build fix
- Fetching corp assets via esi
- Fetching corp contracts via esi
- Fetching corp wallet journal via esi
- Fetching wallet transactions via esi
- Fetching journal until target id hit
- Importing corp market orders via esi
- New scopes
- Fetching conq stations via esi
- Crash fix

fixes #370
- Add character button
- Crash fix
- Removed all manual oauth handling
- Version bump
- Using qnetworkauth for esi

not working because of https://bugreports.qt.io/browse/QTBUG-65778
- Backed out changeset: 1ec4c81cfddb
- Backed out changeset: 2035249cdf59
- Backed out changeset: 27cb05685606
- Backed out changeset: 10778c43568d
- Hack for refresh token bug
- Clearing active refresh tokens
- Missing connection
- Scope fixes
- Opening auth window only once
- Cancelling auth
- Removed manual code input
- Backed out changeset: 229ac8dd338b
- Showing one auth window per char
- Industry mining scope
- Resetting oauth status after access token failure

https://bugreports.qt.io/browse/QTBUG-66097
- Removing unneeded query params
- Properly propagating error
- Verifying character for access code
- Saving refresh tokens
- Properly extracting error message
- Adding query params to get
- Properly parsing errors
- Typo
- Sending type via url query
- Moved contant arguments to concrete class
- Renamed oauth flow to char specific
- Adding new character
- Updated esi endpoints
- Merge with xml-api-removal
- Merge fixes
- Thread-safe db connections
- Fetching ancestries via esi
- 20180214 sde
- Clearing tokens on update
- Removed unused settings
- Changed division numbering to esi-compatible

fixes #402
- Using proper thread pool and futures for async tasks

possible for for db lock

refs #401
- Log esi replies setting
- Crash fix
- Switched verify url to sso server
- Turned off pipelining and spdy, since esi supprots http2
- Warning fix
- Merge
- New esi url
- Separated corp and char pending contract items
- Backed out changeset: 8dfc3f2e0a95
- Copying data for consumer thread
- Literals
- 20180324 sde
- Data is not movable
- Deleting watcher on finish
- String literals
- Switched http2 to pipelining to avoid crashes in qt
- Removing corp contracts from character's
- Possible contract access after destruction
- Thread-safe external order cache
- Http/2 setting
- Brought back translations
- Added tag 3.0-release for changeset b0dbcfe653e3
- Moved tag latest-release to changeset b0dbcfe653e3 (from changeset e82252a670ea)
- Missing mutex
- Recursive external order mutex

fixes #428
- Trans update
- Version bump
- Added tag 3.1-release for changeset 2dc45c51a01e
- Moved tag latest-release to changeset 2dc45c51a01e (from changeset b0dbcfe653e3)
- Some esi debug info
- Removed unused setting
- More info
- Removing impossible results
- Setting corp id in entries

fixes #432
- Showing aggregated data for selection
- Disallowing http/2 for posts because of qt crashes
- Generating win version info
- Proper msvc version check
- SQLITE_LOCKED hack

refs #437
- Bpc flag for blueprints
- Removed question, since http2 seems stable
- Getting solar system id from esi
- Using json version check file
- Removed references to qt xml
- Downloading sde
- Version bump
- Added tag 3.2-release for changeset ad2ad0edd819
- Moved tag latest-release to changeset ad2ad0edd819 (from changeset 2dc45c51a01e)
- New namespaces
- Fetching whole journal/transactions
- Probable final fix for database is locked error
- Rather, this is the fix for database locked
- Busy timeout actually needs to be positive
- Forcing db update when none is present
- Force sde upgrade arg
- Version bump
- Added tag 3.3-release for changeset 1b1ba2dd6d59
- Moved tag latest-release to changeset 1b1ba2dd6d59 (from changeset ad2ad0edd819)
- FromId is actually inclusive
- Fetching paginated wallet journal + version bump
- 20180508 sde
- Some debug
- Token refresh threshold
- Limiting transactions to actually one older than current lowest
- Throttling on 429
- Version bump
- Added tag 3.5-release for changeset dc4d12fb2b20
- Moved tag latest-release to changeset dc4d12fb2b20 (from changeset 1b1ba2dd6d59)
- Not resetting brokers fee on import
- Removing db tokens
- Bpc item flag based on blueprint api

closes #417
- Corp blueprint import
- Removed qt bug workaround
- Removed qt bug workaround
- 20180530 sde
- New esi domain
- Removed conquerable stations
- Added tag 3.6-release for changeset 7170b12f7df4
- Moved tag latest-release to changeset 7170b12f7df4 (from changeset dc4d12fb2b20)
- Option to clear citadels on import
- Manual citadel management

closes #481
- Editing/removing citadels
- Version bump
- Added tag 3.7-release for changeset 8e809123a7d0
- Moved tag latest-release to changeset 8e809123a7d0 (from changeset 7170b12f7df4)
- Updated assets enpoint and setting bpc flag from esi
- Updated order endpoints
- Enforcing reply timeout on the NAM level
- Version bump
- Added tag 3.8-release for changeset afabb2016fb9
- Moved tag latest-release to changeset afabb2016fb9 (from changeset 8e809123a7d0)
- Removed unused crop asset bpc code
- 20180713 sde
- Removed ignore
- 3.9.6 commit
- HotFixes + WIP Migrate to github
- Create main.yml
- Refactoring

### 🚜 Refactor

- Refactored order models
- Refactoring
- Refactored update mechanism (db version still not updating)
- Refactored cache/update timers

### 📚 Documentation

- Dockerfile

### 🎨 Styling

- Style improvement
- Style improvement
- Style improvement
- Style improvement
- Style improvements
- Style improvements
- Style fixes
- Style improvements
- Style fixes
- Style fixes

### 🧪 Testing

- Test output view

### ⚙️ Miscellaneous Tasks

- Citadel sde
- Citadels in station view
- Citadel manager dialog stub
- Citadel view + model
- Citadel warning dialog
- Citadels access denied now has sso status of 0

### ◀️ Revert

- 2328b77e3e61
- Reverted column name
- Reverted debug console

<!-- generated by git-cliff -->
