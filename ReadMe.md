Backblaze dataset native importer [![Unlicensed work](https://raw.githubusercontent.com/unlicense/unlicense.org/master/static/favicon.png)](https://unlicense.org/)
=================================
![GitLab Build Status](https://gitlab.com/KOLANICH/backblaze_analytics_native_importer/badges/master/pipeline.svg)
[![TravisCI Build Status](https://travis-ci.org/KOLANICH/backblaze_analytics_native_importer.svg?branch=master)](https://travis-ci.org/KOLANICH/backblaze_analytics_native_importer)
![GitLab Coverage](https://gitlab.com/KOLANICH1/backblaze_analytics_native_importer/badges/master/coverage.svg)
[![Coveralls Coverage](https://img.shields.io/coveralls/KOLANICH/backblaze_analytics_native_importer.svg)](https://coveralls.io/r/KOLANICH/backblaze_analytics_native_importer)
[![Libraries.io Status](https://img.shields.io/librariesio/github/KOLANICH/backblaze_analytics_native_importer.svg)](https://libraries.io/github/KOLANICH/backblaze_analytics_native_importer)

It is an application written in C++ which should import and normalize BackBlaze dataset into an SQLite DB faster than `backblaze_analytics`. `backblaze_analytics` importer written in python + SQLite is damn slow because SQLite is not very smart and uses journal extensively.

How to build
============

In order to build it you need CMake and the `backblaze_analytics`.

1. `python -m backblaze_analytics import generateC++Schema`
2. Then run CMake
3. Then build.