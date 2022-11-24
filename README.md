# taxme

Taxme is tool for calculating gains and losses on trades using FIFO method for tax purposes. Taxme does not implement tax code for any country, that is up to user to determine from calculated gains and losses.

## Usage

```
taxme year tax/dry DataLabels.cfg asset1.csv asset2.csv ...
```

so for example:

```
taxme 2018 tax DataLabels.cfg BTC.csv ETH.csv
```

which depending on input data may output for example (income - cost = gain/loss):

> Gains for BTC: 1600 - 1200 = 400  
> Gains for ETH: 5850 - 6000 = -150  
> Total gains for year 2018: 7450 - 7200 = 250

## Input files

Taxme works with data provided as csv files. Each csv file represents single asset (e.g. bitcoin, gold, Apple stock). Taxme works with multiple csv files in single invocation. This is helpful when figuring out overall gain or loss from assets, whose gains or losses can be counted against each other. For example, assume that the country's tax code you are interested in allows to count gains and losses from various cryptocurrencies against each other. You might invoke taxme with csv file for BTC and csv file for ETH to figure out not only gains or losses on BTC and ETH, but also overall gain or loss from BTC and ETH combined.

### Asset data - csv file contents

Asset file contains entries which represent change in amount of asset held. Entries are typically trades, but can also represent gifts and other events which user decides to account for. Example file containing entries on single asset may look like this (formatted for display, not actual contents of csv file):

|date|type|unit price in USD|quantity|quantity sold|gain / loss in USD (FIFO)|tax already reported|transaction origin|ID|note|
|---|---|---|---|---|---|---|---|---|---|
|2017-01-31|buy|4000|2|2|n/a|n/a|Coinbase|57fe||
|2017-02-18|buy|6000|1|0.7|n/a|n/a|Coinbase|513ef||
|2017-04-08|sell|3000|1|n/a|-1000|yes|GDAX|687e||
|2017-09-27|sell|5000|1.5|n/a|500|yes|Kraken|45fa|exchange BTC to ETH|
|2018-01-15|sell|8000|0.2|n/a|400|no|GDAX|13ce||
|2018-05-17|buy|7000|1|0|n/a|n/a|Kraken|87fe|exchange ETH to BTC|
|2018-08-14|buy|0|0.01|0|n/a|n/a|Coinbase|refxxx|gift - bonus for signup with referral|

Required columns for each entry are date (in yyyy-mm-dd format), type (buy or sell), unit price and quantity bought or sold. Entries are expected to be sorted by date from oldest to newest in order for FIFO method to work correctly.

Data calculated by taxme are quantity sold (for buys only), gain / loss (for sells only, calculated by FIFO method) and tax already reported (for sells only, yes or no, serves as simple check mark to keep track of entries already reported for tax). These are filled in as output of taxme and can be left blank in input data.

All other columns are optional and taxme does not touch them. These can contain some relevant information for book keeping, such as exchange on which trade was done, trade ID, note etc.

### Asset data labels configuration

Since asset data are meant to be human readable to serve as official records for book keeping by users, configuration of data labels must be passed to taxme in order for taxme to understand them. For example some user might want to use *unit price in USD* and *quantity* as column labels and *buy* and *sell* as labels for types of entries, while another user might want to label their data with *unit price (EUR)*, *volume*, *B* and *S* instead and yet another user might use completely different language. Configuration of data labels looks like this (values on the right side of `=` are user choice and must fit labels used in asset files):

```
Date=date
Type=type
Type.Buy=buy
Type.Sell=sell
UnitPrice=unit price in USD
Quantity=quantity
ClosedPositionQuantity=quantity sold
Gain=gain / loss in USD (FIFO)
IsTaxed=tax already reported
Yes=yes
No=no
NotAvailable=n/a
```

## Output files

For each input asset file `assetX.csv` a new asset file `assetXUpdated.csv` is generated, which contains updated asset file with output columns (quantity sold, gain / loss, tax already reported) recalculated. Input files are used in read only mode and are not modified in any way. All values in output columns are always fully recalculated from values in input columns (date, type, unit price, quantity) and with exception of *tax already reported* column, taxme does not reuse any values in output columns. So for example, when some previous entry is manually corrected, simply running taxme again will recalculate all output columns accordingly.

## Input arguments

There are two input arguments before providing data labels configuration file and list of asset files - year and mode.

```
taxme year mode DataLabels.cfg asset1.csv asset2.csv ...
```

### Year

Selects year for which gains and losses are displayed. So only sell type entries (taxable events) in given year affect displayed output. So typically when figuring out taxes for year *Y*, *Y* would be passed in as year. Note that this doesn't affect which entries have values in output columns in generated output asset files recalculated. All entries are always updated in generated output asset files.

### Mode

Mode is either `tax` or `dry`. Mode only affects column *tax already reported* in generated output asset files.

#### Tax mode

Tax mode is intended for calculating overall gains and losses for given year with intention of reporting them for tax purposes. It marks sell type entries in given year as reported for tax in generated output asset files.

#### Dry mode

Dry mode is intended for simply calculating gains and losses and updating asset files for any use without intention of reporting for tax purposes. It does not mark any new sell type entries as reported for tax in generated output asset files. It can be used for example for figuring out ongoing gains and losses throughout the year or updating files with new entries for book keeping.

## How to compile

```
g++ -O2 -o taxme common.cpp Asset.cpp taxme.cpp
```
