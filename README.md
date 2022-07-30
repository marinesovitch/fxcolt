# fxcolt

## Introduction

The project demonstrates how to implement in C++ a [MetaTrader Expert Advisor](https://www.metatrader4.com/en/trading-platform/help/autotrading/experts) and an external CLI application communicating with it. In this project, the former one is called the 'fxcolt-ea', and the latter the 'kommander-cli'. All components are meant to run on MS Windows.

The fxcolt-ea is implemented in [MQL4](https://www.metaquotes.net/en/metatrader4/automated_trading/mql4) and C++ (dll). It runs under MetaTrader4 and doesn't implement any sort of GUI. The kommander-cli is implemented in C++ and is a stand-alone console application. They communicate between themselves via WinAPI concepts called [mail slots](https://docs.microsoft.com/en-us/windows/win32/ipc/using-mailslots), and [named pipes](https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipes).

### Limitations

The application works with <u>demo accounts only</u>.

<u>**CAUTION! Don't modify sources to use it on real accounts.**</u> It is a proof of concept project, provided as is without thorough testing in real market conditions.

The kommander-cli is meant to work with only one instance of MetaTrader (having the `fxcolt-ea` installed). If there are more such instances running, they may be detected, but the only tested scenario is against a single instance. In the case of communication with more instances, the result is unpredictable.

And the same for the `fxcolt-ea` installed on more charts in the same instance of MetaTrader - the behaviour is unpredictable. The only tested scenario is the `fxcolt-ea` installed on a single chart.

## Sources

I worked on this project in my free time in 2014-2015. Recently, before pushing to GitHub, I refreshed it a little, improved the code slightly, and ported it to the newest Visual Studio 2022 (Community Edition).

### The directory structure

* includes: a bunch of headers consisting of a set of needed `#includes`, used in precompiled headers to not duplicate code.
* cpp: low-level functions and basic structures like string or datetime utilities, thread-safe-queue, streams for logging, etc.
* common: components shared by both `fxcolt-ea` and `kommander-cli`, like functions and data structures related to e.g.: commands, communication (wrappers on mail slots and named pipes), orders, open positions, fx symbols, etc.
* adapter: it is the actual implementation of fxcolt-ea, it contains:
	- [fxcolt.mq4](src/adapter/mql/fxcolt.mq4) which is coded in [MQL4](https://www.metaquotes.net/en/metatrader4/automated_trading/mql4) and is meant to build the `fxcolt.ex4` with [MetaEditor](https://www.metatrader4.com/en/trading-platform/help/autotrading/metaeditor). It imports a lot of functionality from the C++ dll via `#import "mtfxcolt.dll"`. See also [MQL4 Reference](https://docs.mql4.com/).
	- code in C++, it implements the functions imported by `fxcolt.mq4` and is meant to build a `mtfxcolt.dll`
* backend: the implementation of `kommander-cli` backend, there are all data structures and functions needed to manage: command executor, detected fx account, connection, and communication with `fxcolt-ea`, etc.
* kommander: a simple module consisting of the primary function of `kommander-cli` and command loop. It builds binary `kommander.exe`.
* mtstub: a simple replacement for a real MetaTrader used for testing `kommander-cli` in various 'hardcoded' scenarios. It builds binary `mtstub.exe`. To build and/or run it, a separate VS Solution [mtstub.sln](src/mtstub.sln) can be used.

### Dependencies

The only dependency is [the Boost library](https://www.boost.org/). The project was tested against [version 1.79.0](https://www.boost.org/users/history/version_1_79_0.html). Only headers are needed.

It is expected to be located under subdirectory [3rdParty/boost](3rdParty/boost).

The dependency was added as [a git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules). To initialize it, call the script [init_boost.bat](init_boost.bat).

Optionally, download the specified boost version, and unpack it to a mentioned subdirectory (so as the version header is located under *3rdParty/boost/boost/version.hpp*).

## How to build

### kommander-cli

*Build in Visual Studio:*
* open [fxcolt.sln](src/fxcolt.sln) in Visual Studio (tested on VS2022 Community Edition)
* choose the desired configuration: Debug or Release, the only supported platform is Win32
* build, depending on the chosen setup, the binaries will be stored in `bin/Debug` or `bin/Release` (e.g. `bin/Debug/kommander.exe`, `bin/Release/mtfxcolt.dll`, etc.)

### fxcolt-ea

*Build from command line:*
* go to MetaTrader directory (e.g. `C:\Program Files (x86)\MetaTrader4`)
* compile [fxcolt.mq4](src/adapter/mql/fxcolt.mq4) with [MetaEditor](https://www.metatrader4.com/en/trading-platform/help/autotrading/metaeditor)

e.g.
```bat
cd "C:\Program Files (x86)\MetaTrader4"
metaeditor.exe /compile:c:\fxcolt\src\adapter\mql\fxcolt.mq4 /log
```

*Build from MetaEditor:*
* run [MetaEditor](https://www.metatrader4.com/en/trading-platform/help/autotrading/metaeditor) (it is located in MetaTrader directory e.g. `C:\Program Files (x86)\MetaTrader4\metaeditor.exe`)
* open [fxcolt.mq4](src/adapter/mql/fxcolt.mq4) with menu option `File|Open` or key shortcut `CTRL+O`
* compile the opened file with menu option `File|Compile` or key shortcut `F7`

*The build result:*
* Regardless of the way of building `fxcolt.mq4`, the final result will be a file `fxcolt.ex4`. It will be located in the same directory as the source file.

## How to install

### MetaTrader Data Folder

The fxcolt-ex must be installed in the MetaTrader Data Folder. To find it, execute the following steps:
* open MetaTrader
* execute command from menu `File|Open Data Folder`
* it will open the proper directory in the File Explorer
* copy the path, it may look like `C:\Users\<your-user-name>\AppData\Roaming\MetaQuotes\Terminal\<broker-id>` where `broker-id` is a long 32 characters string e.g. 7F5A56A1C319F30813E27B1211E014E9 (it is a fake value and will differ on your machine)

### Install fxcolt-ea

```bat
rem having the path to MetaTrader Data Folder (see the previous point), assign it to MT_DATA_FOLDER environment
rem variable (below there is a fake path for sake of this script, yours will be different)
set MT_DATA_FOLDER=C:\Users\foo\AppData\Roaming\MetaQuotes\Terminal\7F5A56A1C319F30813E27B1211E014E9

rem set the path to sources on your machine
set SRC_ROOT=c:\fxcolt

rem set the configuration Debug or Release
set VS_CONFIGURATION=Debug

rem the directory where VS stores built binaries
set BIN_DIR=%SRC_ROOT%\bin\%VS_CONFIGURATION%

rem create links to fxcolt-ea in the subdirectory of MetaTrader Data Folder meant for Expert Advisors
mklink %MT_DATA_FOLDER%\MQL4\Experts\fxcolt.ex4 %SRC_ROOT%\src\adapter\mql\fxcolt.ex4
mklink %MT_DATA_FOLDER%\MQL4\Libraries\mtfxcolt.dll %BIN_DIR%\mtfxcolt.dll

rem optionally, instead of creating links, the mentioned files can be just copied to the related directories
rem although, they have to be copied every time they are changed, e.g. after each build
copy %SRC_ROOT%\src\adapter\mql\fxcolt.ex4 %MT_DATA_FOLDER%\MQL4\Experts\fxcolt.ex4
copy %BIN_DIR%\mtfxcolt.dll %MT_DATA_FOLDER%\MQL4\Libraries\mtfxcolt.dll
```

### Add fxcolt-ea to a chart in the MetaTrader GUI

* Open MetaTrader
* Open Navigator - from menu `View|Navigator` or key shortcut `Ctrl+N`
* if fxcolt-ea was properly installed in the previous point, then `fxcolt` will be visible in the list of Navigator items under section `Expert Advisors`
* open any chart (e.g. for symbol EURUSD)
* add `fxcolt` to the chosen chart via the context menu option `Attach to a chart` or with drag&drop
* [Click here to see how the installed fxcolt-ea looks like in the MetaTrader GUI](https://raw.githubusercontent.com/marinesovitch/media/trunk/fxcolt/fxcolt-ea.png)

## How to use

### List of commands

| Command         | Alias | Description                             | Parameters                        |
| :---:           | :---: | :---                                    | :---                              |
| help            | h     | print list of available commands        | *no params*                       |
| accounts        | la    | print list of available MT accounts     | *no params*                       |
| select          | sel   | select active account                   | `account-index`                   |
| get_symbols     | gs    | get registered symbols                  | *no params*                       |
| list_symbols    | ls    | list all available symbols              | *no params*                       |
| get             | g     | get order(s) by ID                      | `[order-id...]`                   |
| open            | o     | open a new order | `symbol order-type lots open-price [SL-price] [TP-price] [expiration-timestamp]` |
| modify          | m     | modify an existing order | `open-price SL-price TP-price expiration-timestamp order-id [order-id...]` |
| set_stop_loss   | sl    | set SL (stop-loss) for given order(s)   | `SL-price order-id [order-id...]` |
| set_take_profit | tp    | set TP (take-profit) for given order(s) | `TP-price order-id [order-id...]` |
| show_ticks      | st    | print ticks for a registered symbol     | *no params*                       |
| hide_ticks      | ht    | stop printing ticks                     | *no params*                       |
| close           | c     | close specified order(s)                | `order-id [order-id...]`          |
| close_all       | ca    | close all open and pending orders       | *no params*                       |
| exit            |       | exit commander-cli                      | *no params*                       |

### Command reference

To run a command there can be used its full name or an alias.

---------------

*help (h)*

Print the list of available commands. In each row, there is a full name and its alias in parenthesis.

Samples:

```bat
$ h
help (h)
accounts (la)
select (sel)
close (c)
close_all (ca)
get (g)
get_symbols (gs)
hide_ticks (ht)
list_symbols (ls)
modify (m)
open (o)
set_stop_loss (sl)
show_ticks (st)
set_take_profit (tp)
exit
```

---------------

*accounts (la)*

Print the list of all available MetaTrader accounts with properly installed fxcolt-ea.

Samples:

```bat
$ la
0 Biuro_maklerskie_mBanku 9876543210
```

---------------

*select (sel)*

Select the active account from a list of accounts retrieved with the command `accounts`.

Samples:

```bat
$ accounts
0 Biuro_maklerskie_mBanku 9876543210
$ sel 0
0 Biuro_maklerskie_mBanku 9876543210
```

---------------

*get_symbols (gs)*

Get registered symbol. It is the symbol for which chart the `fxcolt-ea` was added. The application was tested only against a single registered symbol. In the case of more ones, the behaviour is unpredictable.

Samples:

```bat
$ gs
EURUSD
```

---------------

*list_symbols (ls)*

List all available symbols for the active account.

Samples:

```bat
$ ls
$ KTrader::onCmdResult cmd sent successfully
KTrader::onSymbol USDCHF
KTrader::onSymbol GBPUSD
KTrader::onSymbol EURUSD
KTrader::onSymbol USDJPY
KTrader::onSymbol USDCAD
KTrader::onSymbol AUDUSD
KTrader::onSymbol EURGBP
KTrader::onSymbol EURAUD
KTrader::onSymbol EURCHF
KTrader::onSymbol GBPCHF
KTrader::onSymbol CADJPY
KTrader::onSymbol GBPJPY
KTrader::onSymbol AUDNZD
KTrader::onSymbol AUDCAD
KTrader::onSymbol AUDCHF
KTrader::onSymbol AUDJPY
KTrader::onSymbol CHFJPY
KTrader::onSymbol EURNZD
KTrader::onSymbol EURCAD
KTrader::onSymbol CADCHF
KTrader::onSymbol NZDJPY
KTrader::onSymbol NZDUSD
KTrader::onSymbol EU50.f
KTrader::onSymbol DE30.f
KTrader::onSymbol POL20.f
KTrader::onSymbol US500.f
KTrader::onSymbol CHFPLN
KTrader::onSymbol USDPLN
KTrader::onSymbol GBPPLN
KTrader::onSymbol WTI.f
KTrader::onSymbol GOLD.f
KTrader::onSymbol EURPLN
KTrader::onSymbol EURJPY
```

---------------

*get (g)*

Get order(s) by ID. If no ID was provided, then all open and/or pending orders will be listed.

Get order output legend:

| Symbol | Order Id | Status | Type | Lots | Open price | Close price | SL    | TP   | Open Timestamp | Expiration | Close Timestamp | Commission | Swap | Profit |
| :---   | :---     | :---   | :--- | ---: |       ---: |        ---: |  ---: | ---: |           ---: |       ---: |            ---: |       ---: | ---: |   ---: |
| EURUSD | 13860828 | Open   | Buy  |  0.1 |    1.02174 |     1.02171 | 1.011 | 1.07 |     1658778969 |          0 |               0 |       -3.3 |    0 |  -1.39 |

Order status:
- Pending
- Open
- Closed

Order type:
- Buy
- Sell
- BuyLimit
- SellLimit
- BuyStop
- SellStop


Samples:

```bat
$ g 13860828
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860828 Open Buy 0.1 1.02174 1.02171 1.011 1.07 1658778969 0 0 -3.3 0 -1.39
```

```bat
g 13860807
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860807 Closed Sell 0.2 1.02245 1.02233 1.06 0.97 1658776746 0 1658785888 -6.61 0 11.09
```

```bat
$ get 13860817 13860819
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860817 Pending BuyLimit 0.1 1.0217 1.02184 0.99 1.05 1658777289 0 0 0 0 0
KTrader::onOrder EURUSD 13860819 Pending BuyLimit 0.1 1.02171 1.02184 0.97 1.06 1658777335 0 0 0 0 0
```

```bat
$ g
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860803 Open Buy 0.1 1.02251 1.02166 0.9583 1.05 1658776653 0 0 -3.3 0 -39.27
KTrader::onOrder EURUSD 13860806 Open Buy 0.2 1.02251 1.02166 0.98 0 1658776716 0 0 -6.61 0 -78.53
KTrader::onOrder EURUSD 13860807 Open Sell 0.2 1.02245 1.0217 1.06 0.97 1658776746 0 0 -6.61 0 69.31
KTrader::onOrder EURUSD 13860811 Open Sell 0.1 1.02234 1.0217 1.087 0.954 1658776894 0 0 -3.3 0 29.57
KTrader::onOrder EURUSD 13860817 Open Buy 0.1 1.0217 1.02166 0.99 1.05 1658778643 0 0 -3.3 0 -1.85
KTrader::onOrder EURUSD 13860819 Open Buy 0.1 1.02171 1.02166 0.97 1.06 1658778631 0 0 -3.3 0 -2.31
KTrader::onOrder EURUSD 13860823 Pending SellLimit 0.1 1.03 1.02166 1.0534 0.99 1658777422 0 0 0 0 0
KTrader::onOrder EURUSD 13860824 Pending SellLimit 0.1 1.0278 1.02166 1.0334 0.9871 1658777466 0 0 0 0 0
KTrader::onOrder EURUSD 13860828 Pending BuyStop 0.1 1.02173 1.0217 1.011 1.07 1658777572 0 0 0 0 0
KTrader::onOrder EURUSD 13860830 Pending BuyStop 0.1 1.0321 1.0217 1.0253 1.0428 1658777634 0 0 0 0 0
KTrader::onOrder EURUSD 13860834 Open Sell 0.1 1.02135 1.0217 1.04 0.99 1658778775 0 0 -3.3 0 -16.18
KTrader::onOrder EURUSD 13860835 Pending SellStop 0.1 1.0178 1.02166 1.0256 0 1658777774 0 0 0 0 0
KTrader::onOrder EURUSD 13860837 Pending SellStop 0.2 0.9878 1.02166 1.0367 0.9791 1658777843 0 0 0 0 0
KTrader::onOrder EURUSD 13860847 Pending BuyLimit 0.1 1.019 1.0217 0 0 1658778188 0 0 0 0 0
```

---------------

*open (o)*

Open a new order.

Syntax:
`symbol order-type lots open-price [SL-price] [TP-price] [expiration-timestamp]`

| Option               | Description                                       | Optional |
| :---                 | :---                                              |  :---:   |
| symbol               | a symbol like EURUSD, AUDUSD, EURGBP, CHFPLN etc. |   No     |
| order-type           | see the table below                               |   No     |
| lots                 | size of order in lots e.g. 0.1 2.0                |   No     |
| open-price           | an open price like e.g. 1.0213 4.321 etc.         |   No     |
| SL-price             | a stop loss price, 0 means no SL                  |   Yes    |
| TP-price             | a take profit price, 0 means no TP                |   Yes    |
| expiration-timestamp | expiration timestamp, 0 means no timestamp        |   Yes    |

The order type can be used either as a full name or as an alias:

| Full Name  | Alias |
| :--------- | :---: |
| Buy        |   b   |
| Sell       |   s   |
| BuyLimit   |   bl  |
| SellLimit  |   sl  |
| BuyStop    |   bs  |
| SellStop   |   ss  |

If the order is correct, then it will be accepted, and its parameters will be logged after notification. The output is identical to the `get` command.


Samples:

```bat
$ o EURUSD Buy 0.1 1.0227 0.98 1.05
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860803 Open Buy 0.1 1.02251 1.02247 0.98 1.05 1658776653 0 0 -3.3 0 -1.85

$ open EURUSD BuyLimit 0.1 1.01 0.94 1.0545
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860817 Pending BuyLimit 0.1 1.01 1.02236 0.94 1.0545 1658777289 0 0 0 0 0

$ open EURUSD bl 0.1 1.001
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860819 Pending BuyLimit 0.1 1.001 1.02235 0 0 1658777335 0 0 0 0 0

$ open EURUSD SellLimit 0.1 1.03 1.0534 0.99
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860823 Pending SellLimit 0.1 1.03 1.02255 1.0534 0.99 1658777422 0 0 0 0 0

$ o EURUSD sl 0.1 1.0278 1.0334 0.9871
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860824 Pending SellLimit 0.1 1.0278 1.02232 1.0334 0.9871 1658777466 0 0 0 0 0

$ open EURUSD BuyStop 0.1 1.03
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860828 Pending BuyStop 0.1 1.03 1.02206 0 0 1658777572 0 0 0 0 0

$ open EURUSD bs 0.1 1.03210 1.0253 1.0428
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860830 Pending BuyStop 0.1 1.0321 1.02205 1.0253 1.0428 1658777634 0 0 0 0 0

$ open EURUSD SellStop 0.1 0.9978 1.01534 0.9712
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860834 Pending SellStop 0.1 0.9978 1.02207 1.01534 0.9712 1658777736 0 0 0 0 0

$ open EURUSD ss 0.1 1.0178 1.0256
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860835 Pending SellStop 0.1 1.0178 1.02206 1.0256 0 1658777774 0 0 0 0 0
```

---------------

*modify (m)*

Modify existing order(s).

Syntax:
`open-price SL-price TP-price expiration-timestamp order-id [order-id...]`

| Option                   | Description                                |
| :---                     | :---                                       |
| `open-price`             | an open price like e.g. 1.0213 4.321 etc.  |
| `SL-price`               | a stop loss price, 0 means no SL           |
| `TP-price`               | a take profit price, 0 means no TP         |
| `expiration-timestamp`   | expiration timestamp, 0 means no timestamp |
| `order-id [order-id...]` | one or more order IDs                      |

Samples:

```bat
m 1.0214 1.04 0.99 0 13860834
$ KTrader::onCmdResult cmd sent successfully
get 13860834
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860834 Open Sell 0.1 1.02135 1.02141 1.04 0.99 1658778775 0 0 -3.3 0 -2.77
```

```bat
modify 1.0217 0.99 1.05 0 13860817
$ KTrader::onCmdResult cmd sent successfully
m 1.02171 0.97 1.06 0 13860819
$ KTrader::onCmdResult cmd sent successfully
```

---------------

*set_stop_loss (sl)*

Set SL (stop-loss) for given list of order(s).

Syntax:
`SL-price order-id [order-id...]`

| Option                   | Description                      |
| :---                     | :---                             |
| `SL-price`               | a stop loss price, 0 means no SL |
| `order-id [order-id...]` | one or more order IDs            |

Samples:

```bat
$ sl 0.9512 13860806
$ KTrader::onCmdResult cmd sent successfully
$ g 13860806
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860806 Open Buy 0.2 1.02251 1.02194 0.9512 0 1658776716 0 0 -6.61 0 -52.65

$ set_stop_loss 1.0123 13860803
$ KTrader::onCmdResult cmd sent successfully
$ get 13860803
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860803 Open Buy 0.1 1.02251 1.0219 1.0123 1.05 1658776653 0 0 -3.3 0 -28.17
```

---------------

*set_take_profit (tp)*

Set TP (take-profit) for the given list of order(s).

Syntax:
`TP-price order-id [order-id...]`

| Option                   | Description                        |
| :---                     | :---                               |
| `TP-price`               | a take profit price, 0 means no TP |
| `order-id [order-id...]` | one or more order IDs              |

Samples:

```bat
$ tp 1.04545 13860806
$ KTrader::onCmdResult cmd sent successfully
$ get 13860806
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860806 Open Buy 0.2 1.02251 1.02197 0.9512 1.04545 1658776716 0 0 -6.61 0 -49.87

$ set_take_profit 1.08 13860828
$ KTrader::onCmdResult cmd sent successfully
$ g 13860828
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860828 Open Buy 0.1 1.02174 1.02176 1.011 1.08 1658778969 0 0 -3.3 0 0.92
```

---------------

*show_ticks (st)*

Start printing ticks for a registered symbol (see command `get_symbols`). Depending on the market activity, the ticks may flood the console.

Tick output legend:

| Label  | Symbol | Ask price | Bid price | Last deal price | Timestamp  |
| :---   | :---   |      ---: |      ---: |            ---: |       ---: |
| onTick | EURUSD |   1.02265 |   1.02261 |               0 | 1658776477 |

See also [MqlTick Structure Reference](https://docs.mql4.com/constants/structures/mqltick)

Samples:

```bat
$ st
$ onTick EURUSD 1.02265 1.02261 0 1658776477
onTick EURUSD 1.02266 1.02262 0 1658776477
onTick EURUSD 1.02265 1.02261 0 1658776477
onTick EURUSD 1.02265 1.0226 0 1658776478
onTick EURUSD 1.02264 1.02261 0 1658776478
onTick EURUSD 1.02267 1.02262 0 1658776480
onTick EURUSD 1.02265 1.02262 0 1658776480
onTick EURUSD 1.02266 1.02261 0 1658776481
onTick EURUSD 1.02265 1.02261 0 1658776481
onTick EURUSD 1.02266 1.02261 0 1658776481
onTick EURUSD 1.02266 1.02262 0 1658776482
onTick EURUSD 1.02265 1.0226 0 1658776482
```

---------------

*hide_ticks (ht)*

Stop printing ticks. While ticks are printed just input 'ht' or 'hide_ticks' and press Enter. The ticks may cut in between the command letters, but it doesn't matter. See the sample output - the letters 'h' and 't' are at the beginning of the last two lines. Regardless of the ticks, the command is processed properly.

Samples:

```bat
onTick EURUSD 1.02266 1.02262 0 1658776483
honTick EURUSD 1.02266 1.02263 0 1658776483
tonTick EURUSD 1.02266 1.02262 0 1658776483
```
---------------

*close (c)*

Closes specified list of order(s). It closes at least one order.

Syntax:
`order-id [order-id...]`

| Option                   | Description           |
| :---                     | :---                  |
| `order-id [order-id...]` | one or more order IDs |

Samples:

```bat
$ c 13860807
$ KTrader::onCmdResult cmd sent successfully
$ g 13860807
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860807 Closed Sell 0.2 1.02245 1.02233 1.06 0.97 1658776746 0 1658785888 -6.61 0 11.09

$ close 13860803 13860823 13860834
$ KTrader::onCmdResult cmd sent successfully
$ get 13860803 13860823 13860834
$ KTrader::onCmdResult cmd sent successfully
KTrader::onOrder EURUSD 13860803 Closed Buy 0.1 1.02251 1.02234 1.0123 1.05 1658776653 0 1658785965 -3.3 0 -7.85
KTrader::onOrder EURUSD 13860823 Closed SellLimit 0.1 1.03 1.02234 1.0534 0.99 1658777422 0 1658785965 0 0 0
KTrader::onOrder EURUSD 13860834 Closed Sell 0.1 1.02135 1.02239 1.04 0.99 1658778775 0 1658785965 -3.3 0 -48.03
```

---------------

*close_all (ca)*

Closes all open and pending orders.

**CAUTION!** It doesn't ask for confirmation. Once again, it is a proof of concept project meant for use with the demo accounts only.

Samples:

```bat
$ close_all
$ KTrader::onCmdResult cmd sent successfully
```

```bat
$ ca
$ KTrader::onCmdResult cmd sent successfully
```

---------------

*exit*

It closes the session and ends the kommander-cli process.

---------------

### A sample session log

[Click here to see a sample kommander-cli session](https://github.com/marinesovitch/media/blob/trunk/fxcolt/kommander-cli-session.log).

[Click here to see the MetaTrader screenshot with the list of orders opened during that session](https://raw.githubusercontent.com/marinesovitch/media/trunk/fxcolt/orders.png).

[Click here to see the MetaTrader screenshot with the list of orders closed before ending the session](https://raw.githubusercontent.com/marinesovitch/media/trunk/fxcolt/closed_orders.png).

---------------

### Diagnostics

The fxcolt-ea logs diagnostic messages into `c:/fxcolt/session.log`. The path is hardcoded in [`src/adapter/detail/dllmain.cpp`](src/adapter/detail/dllmain.cpp). The log file will be created only in case the directory `c:/fxcolt` exists.

The messages are generated in [fxcolt.mq4](src/adapter/mql/fxcolt.mq4) with functions:
* void LogWrite(string msg)
* void LogWriteln(string msg)

At each start of MetaTrader the log will be <u>overwritten</u>. The previous contents will be <u>lost</u>.

[Click here to see a sample fxcolt-ea session.log](https://github.com/marinesovitch/media/blob/trunk/fxcolt/fxcolt-ea-session.log).
