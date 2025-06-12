#!/bin/bash

PORT=8081
BASE="http://localhost:$PORT"

echo "🔍 Testing HTTP methods..."

# GET request
echo -e "\n▶️ GET /"
curl -i "$BASE/"

# POST request
echo -e "\n▶️ POST /upload/"
curl -i -X POST -F "file=@test.txt" "$BASE/upload/"

# PUT request
echo -e "\n▶️ PUT /upload/put.txt"
curl -i -X PUT --data 'sample data' "$BASE/upload/put.txt"

# DELETE request
echo -e "\n▶️ DELETE /upload/put.txt"
curl -i -X DELETE "$BASE/upload/put.txt"

echo -e "\n✅ Done testing HTTP methods."