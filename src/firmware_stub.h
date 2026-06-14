#pragma once

// ---------------------------------------------------------------------------
// firmware_stub.h — Hardware Abstraction Layer (HAL) stub
//
// Purpose
// ───────
// In a production 5G small cell the agent runs on an embedded Linux board
// (e.g. Qualcomm FSM100xx, NXP Layerscape).  This stub documents the
// register-level and GPIO interfaces the agent would use on real hardware,
// and provides safe software-only fallbacks so the same codebase compiles
// and runs on a development host.
//
// Cross-compilation note
// ──────────────────────
// The CMakeLists.txt (TARGET_ARCH option) can target:
//   native  →  x86-64  (default, this file)
//   aarch64 →  ARM64   (production board)
// When building for aarch64, define EMBEDDED_HW to enable the real
// memory-mapped register access below.
// ---------------------------------------------------------------------------

#include <cstdint>
#include <string>

// ── Register map (illustrative — matches a typical small cell SoC) ─────────

#ifdef EMBEDDED_HW
// On real hardware these addresses are memory-mapped I/O regions.
// Access requires root or a kernel UIO driver.
static constexpr uintptr_t REG_BASE_RADIO   = 0xFE80'0000UL;
static constexpr uintptr_t REG_CELL_STATUS  = REG_BASE_RADIO + 0x0004;
static constexpr uintptr_t REG_SINR_RAW     = REG_BASE_RADIO + 0x0010;
static constexpr uintptr_t REG_RSRP_RAW     = REG_BASE_RADIO + 0x0014;
static constexpr uintptr_t REG_RSRQ_RAW     = REG_BASE_RADIO + 0x0018;
static constexpr uintptr_t REG_TX_POWER     = REG_BASE_RADIO + 0x0020;
#endif

// ── GPIO pin definitions ────────────────────────────────────────────────────
static constexpr int GPIO_CELL_ACTIVE_LED  = 42;   // Green LED — cell ACTIVE
static constexpr int GPIO_ALERT_LED        = 43;   // Red LED   — alert active
static constexpr int GPIO_SSH_ENABLE       = 44;   // SSH enable jumper

// ── HAL interface ───────────────────────────────────────────────────────────

namespace hal {

// Read a 32-bit hardware register.
// On a development host returns a safe default (0).
inline uint32_t readRegister([[maybe_unused]] uintptr_t address) {
#ifdef EMBEDDED_HW
    volatile auto* reg = reinterpret_cast<volatile uint32_t*>(address);
    return *reg;
#else
    return 0u;   // software fallback
#endif
}

// Write a 32-bit hardware register.
inline void writeRegister([[maybe_unused]] uintptr_t address,
                          [[maybe_unused]] uint32_t  value) {
#ifdef EMBEDDED_HW
    volatile auto* reg = reinterpret_cast<volatile uint32_t*>(address);
    *reg = value;
#endif
}

// Set a GPIO pin (Linux sysfs interface or direct register on bare-metal).
inline void gpioWrite([[maybe_unused]] int pin,
                      [[maybe_unused]] bool high) {
#ifdef EMBEDDED_HW
    // sysfs: echo high ? "1" : "0" > /sys/class/gpio/gpio<pin>/value
    // Omitted here to keep the stub header-only.
#endif
}

// Return a human-readable board description.
inline std::string boardDescription() {
#ifdef EMBEDDED_HW
    return "ARM64 small cell SoC (production)";
#else
    return "x86-64 development host (HAL stub)";
#endif
}

} // namespace hal
