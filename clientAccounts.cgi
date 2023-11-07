#!/bin/bash

# Include the HTTP header
echo "Content-type: application/json"
echo ""

# Get the client parameter from the query string
client=$(echo "$QUERY_STRING" | sed -n 's/^.*client=\([^&]*\).*$/\1/p')

JSON_FILE="/fibudata/${client}_accounts.json"

# Determine if the request is GET or POST
if [ "$REQUEST_METHOD" = "GET" ]; then
  if [ -f "$JSON_FILE" ]; then
    cat "$JSON_FILE"
  else
    echo -n "[]"
  fi
elif [ "$REQUEST_METHOD" = "POST" ]; then
  # Read POST data
  read -r data
  echo "$data" > "$JSON_FILE"
fi

