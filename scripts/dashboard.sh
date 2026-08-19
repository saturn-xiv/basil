#!/bin/bash

set -e

# https://vite.dev/guide/#scaffolding-your-first-vite-project
npm install --save bulma \
    react-router react-intl \
    @graphiql/react \
    dayjs \
    @reduxjs/toolkit react-redux \
    js-cookie @types/js-cookie jwt-decode

exit 0
