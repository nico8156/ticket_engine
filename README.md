# TicketVerify Engine

TicketVerify Engine is a **stateless C++20 CLI** that parses **raw OCR text** (iOS Vision / ML Kit) from café/restaurant receipts and outputs **structured JSON v1** with **status + confidence**.

## Features (MVP)
- OCR normalization (robust to real-world noise)
- Merchant extraction (multi-line headers)
- Total TTC extraction (handles multi-line amounts)
- Signals detection: **card keywords**, **TVA**, **SIRET**
- Unit tests + real OCR fixture integration test

## Build
```bash
cmake -S . -B build
cmake --build build

