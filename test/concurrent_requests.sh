#!/bin/bash

echo "🚀 Launching 5 concurrent clients..."
for i in {1..5}
do
  echo "Starting image client $i..."
  # Start the client in the background
  ./client_images &
  echo "Starting text client $i..."
  ./client_texts &
done

wait
echo "✅ All concurrent clients finished."