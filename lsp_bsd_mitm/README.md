# lsp_bsd_mitm

Atmosphere **bsd:u** MITM sysmodule that logs Mario Kart Live Home Circuit Fuji LSP
socket traffic to the SD card. Built for **Atmosphere 1.11.x** (FW 22.x).

This hooks the same layer we discussed — not `network_mitm` (SSL-only).

## Output

| File | Contents |
|------|----------|
| `sdmc:/lsp_mitm/capture.bin` | Length-prefixed records (Fuji ports only) |
| `sdmc:/lsp_mitm/lsp_bsd_mitm.log` | Sysmodule log |

### capture.bin record format

```
[0]     dir     0=tx 1=rx
[1]     proto   0=tcp 1=udp
[2:4]   port    big-endian local Fuji port (5016+id, 5032+id, 5116+id, 5201/5202)
[4:8]   len     big-endian payload length
[8:12]  reserved (0)
[12:]   payload
```

Logged ports: **5016–5024**, **5032–5040**, **5116–5124**, **5201**, **5202**.

Game filter (hard-coded): MKLHC base `0100ED100BA3A000`, update `0100ED100BA3A800`.

## Install

1. Build (see below) or copy `out/sd/atmosphere/contents/4200000000000667/` to SD.
2. Reboot Switch (CFW).
3. Launch MKLHC, connect kart, drive a session.
4. Pull `sdmc:/lsp_mitm/capture.bin`.

Sysmodule title ID: `4200000000000667`.

## Build

Requires Docker (recommended):

```bash
cd switch/lsp_bsd_mitm
git submodule update --init --recursive
docker compose up --build
```

Artifact: `out/lsp_bsd_mitm-0.1.0.zip` (ready-to-copy `atmosphere/` tree).

Native devkitPro:

```bash
cd switch/lsp_bsd_mitm
git submodule update --init --recursive
make dist
```

## Decode on PC

```bash
python3 switch/lsp_bsd_mitm/tools/decode_capture.py capture.bin | head
```

Look for:

- TCP **5032+id**: `PI` (50 49) clock (18-byte multiples), kart clock first
- UDP **5016+id**: `ec c1` FRAM video datagrams
- First FRAM payloads should contain SPS (`67`) on a working Switch session

## Notes

- Intercepts **MKLHC only** (whitelist). Second BSD session per process is mitm'd (first is skipped — same as ryu_ldn_nx).
- High-volume UDP video fills the capture quickly; pull the file after a short test.
- Based on bsd MITM forwarder from [ryu_ldn_nx](https://github.com/Ethiquema/ryu_ldn_nx) (GPL-2.0); proxy paths remain but MKLHC traffic uses plain forward.

## License

GPL-2.0-or-later (sysmodule). See `LICENSE` if vendored from upstream.
