#!/usr/bin/env bash
set -euo pipefail

HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-5000}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BIN_DIR="${PROJECT_ROOT}/bin"

SERVER_BIN="${BIN_DIR}/rtype_server"
CLIENT_BIN="${BIN_DIR}/rtype_client"

SERVER_PID=""
CLIENT1_PID=""
CLIENT2_PID=""

cleanup() {
  echo ""
  echo "==> Shutting down..."
  
  if [ -n "$CLIENT1_PID" ] && kill -0 "$CLIENT1_PID" 2>/dev/null; then
    echo "Stopping client 1 (PID: $CLIENT1_PID)"
    kill "$CLIENT1_PID" 2>/dev/null || true
  fi
  
  if [ -n "$CLIENT2_PID" ] && kill -0 "$CLIENT2_PID" 2>/dev/null; then
    echo "Stopping client 2 (PID: $CLIENT2_PID)"
    kill "$CLIENT2_PID" 2>/dev/null || true
  fi
  
  if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Stopping server (PID: $SERVER_PID)"
    kill "$SERVER_PID" 2>/dev/null || true
  fi
  
  wait 2>/dev/null || true
  echo "All processes stopped."
}

trap cleanup EXIT INT TERM

if [ ! -f "$SERVER_BIN" ]; then
  echo "error: Server binary not found at $SERVER_BIN"
  echo "Please run ./scripts/bootstrap.sh first to build the project."
  exit 1
fi

if [ ! -f "$CLIENT_BIN" ]; then
  echo "error: Client binary not found at $CLIENT_BIN"
  echo "Please run ./scripts/bootstrap.sh first to build the project."
  exit 1
fi

# Change to project root so config and assets are found
cd "$PROJECT_ROOT"

echo "==> Starting R-Type server on port $PORT"
"$SERVER_BIN" "$PORT" &
SERVER_PID=$!
echo "Server started (PID: $SERVER_PID)"

echo "==> Waiting for server to initialize..."
sleep 2

echo "==> Starting client 1"
"$CLIENT_BIN" "$HOST" "$PORT" &
CLIENT1_PID=$!
echo "Client 1 started (PID: $CLIENT1_PID)"

sleep 1

echo "==> Starting client 2"
"$CLIENT_BIN" "$HOST" "$PORT" &
CLIENT2_PID=$!
echo "Client 2 started (PID: $CLIENT2_PID)"

echo ""
echo "========================================="
echo "R-Type is running!"
echo "Server:   PID $SERVER_PID on port $PORT"
echo "Client 1: PID $CLIENT1_PID"
echo "Client 2: PID $CLIENT2_PID"
echo "========================================="
echo "Press Ctrl+C to stop all processes"
echo ""

wait
