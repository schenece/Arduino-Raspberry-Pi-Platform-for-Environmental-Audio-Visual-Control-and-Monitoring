#pragma once

/*
 * ========================================================================
 *  FadeTable.h
 *  ----------------------------------------------------------------------
 *  Defines per-track light fade parameters manually tuned to sync with
 *  the duration and character of each audio file.
 *
 *  - Each entry corresponds to a track number (e.g. index 0 = track 0001)
 *  - brightnessStepUs → how quickly brightness changes per update
 *  - cyclesPerStep    → how many AC cycles to hold before stepping again
 *
 *  These parameters control the TRIAC delay time in the dimmer circuit,
 *  producing a light pattern that fades in and out in sync with audio.
 *
 *  🟡 Special case:
 *    - Track 0013 is missing; index 12 is set to {0, 0} and should be skipped.
 *
 *  🛠️  To add/modify:
 *    - Use visual testing script to tune sync
 *    - Edit values directly in this table
 * ========================================================================
 */

struct FadeParams {
  int brightnessStepUs;
  int cyclesPerStep;
};

// Track 13 is missing. So index 12 will be a dummy and skipped.
const FadeParams fadeTable[] = {
  {  56, 2 },  // 0001
  {  46, 2 },  // 0002
  { 154, 6 },  // 0003
  {  98, 5 },  // 0004
  {  70, 4 },  // 0005
  { 148, 5 },  // 0006
  { 124, 7 },  // 0007
  { 120, 8 },  // 0008
  { 120, 7 },  // 0009
  { 120, 9 },  // 0010
  {  90, 7 },  // 0011
  { 117,10 },  // 0012
  {   0, 0 },  // ---- (13 is missing)
  {  87, 8 },  // 0014
  {  55, 4 },  // 0015
  {  80, 3 },  // 0016
  { 120, 3 },  // 0017
  {  75, 3 },  // 0018
  {  54, 4 },  // 0019
  {  30, 2 },  // 0020
  {  55, 5 },  // 0021
  { 118, 9 },  // 0022
  {  29, 2 },  // 0023
  {  88, 3 }   // 0024
};

// Total number of entries for boundary check
const int NUM_TRACKS = sizeof(fadeTable) / sizeof(fadeTable[0]);