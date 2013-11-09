sailsmart
=========

How to get started
------------------

1. Pull the source code
2. View index.html from within a web browser

File summary
------------

    allnavs.js       Recorded measurements over several years in JSON format
    common.js        Common functions used everywhere.
    index.html       The main file.
    jquery.min.js    JQuery library
    makeparser.js    Include this file inside the index.html to generate production rules
                     that can be pasted into http://pegjs.majda.cz/ to generate a parser
                     for a context free grammar.
    matrix.js        A matrix class
    parser002.js     Output from a parser generator
    parserutils.js   Used by makeparser.js
    parsetree.js     Saved output from the parser.
    sailmodel002.m   Defines the statemachine (costs, transitons, etc.)
    sailrecord.js    Routines to parse the data in allnavs.js
    statemachine.js  Solves the HMM using dynamic programming
    test%03d.js      Short scripts to include inside index.html to test/illustrate features.
    treeinfo.js      Code to display the parse tree.
    treeinfo002.js   Code to display the specific parse tree that we define for sailmodel002.

How to
------

... tweak the costs of the HMM:
See SailModel002Settings inside sailmodel002.js

... modify the HMM:
See sailmodel002.m

... modify the context free grammar:
See makeparser.js and treeinfo002.js
