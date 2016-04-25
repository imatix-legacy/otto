!*===========================================================================*
!                                                                            *
!   otto.l      Otto build-script generator 2.0                              *
!                                                                            *
!   Written:    96/09/23  iMatix <otto@imatix.com>                           *
!   Revised:    97/02/26                                                     *
!                                                                            *
!   Copyright (c) 1996-97 iMatix                                             *
!                                                                            *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 2 of the License, or        *
!   (at your option) any later version.                                      *
!                                                                            *
!   This program is distributed in the hope that it will be useful,          *
!   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
!   GNU General Public License for more details.                             *
!                                                                            *
!   You should have received a copy of the GNU General Public License        *
!   along with this program; if not, write to the Free Software              *
!   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
!*===========================================================================*
-schema=lrschema.pl

After-Init:
    (--) Ok                                 -> Have-Argument
          + Initialise-Program-Data
          + Get-Next-Argument-Value
    (--) Error                              ->
          + Terminate-The-Program

Have-Argument:
    (--) Ok                                 -> Have-Command
          + Open-Command-Script
          + Get-Next-Command-Line
    (--) Switch                             -> Have-Argument
          + Process-The-Switch
          + Get-Next-Argument-Value
    (--) Finished                           ->
          + Terminate-The-Program

Have-Command:
    (--) Ignore                             -> Have-Command
          + Get-Next-Command-Line
    (--) Append                             -> Have-Command
          + Process-Append-Command
          + Get-Next-Command-Line
    (--) Call                               -> Have-Command
          + Process-Call-Command
          + Get-Next-Command-Line
    (--) Close                              -> Have-Command
          + Process-Close-Command
          + Get-Next-Command-Line
    (--) Comment                            -> Have-Command
          + Process-Comment-Command
          + Get-Next-Command-Line
    (--) Compile                            -> Have-Command
          + Process-Compile-Command
          + Get-Next-Command-Line
    (--) Copy                               -> Have-Command
          + Process-Copy-Command
          + Get-Next-Command-Line
    (--) Delete                             -> Have-Command
          + Process-Delete-Command
          + Get-Next-Command-Line
    (--) Echo                               -> Have-Command
          + Process-Echo-Command
          + Get-Next-Command-Line
    (--) Install                            -> Have-Command
          + Process-Install-Command
          + Get-Next-Command-Line
    (--) Library                            -> Have-Command
          + Process-Library-Command
          + Get-Next-Command-Line
    (--) Link                               -> Have-Command
          + Process-Link-Command
          + Get-Next-Command-Line
    (--) Output                             -> Have-Command
          + Open-New-Output-File
          + Get-Next-Command-Line
    (--) Pause                              -> Have-Command
          + Process-Pause-Command
          + Get-Next-Command-Line
    (--) Rename                             -> Have-Command
          + Process-Rename-Command
          + Get-Next-Command-Line
    (--) Require                            -> Have-Command
          + Process-Require-Command
          + Get-Next-Command-Line
    (--) System-Specific                    -> Have-Command
          + Process-System-Specific-Command
          + Get-Next-Command-Line
    (--) Title                              -> Have-Command
          + Process-Title-Command
          + Get-Next-Command-Line
    (--) Finished                           -> Have-Argument
          + Close-Command-Script
          + Signal-Script-Processed
          + Get-Next-Argument-Value

Defaults:
    (--) Exception                          ->
          + Terminate-The-Program
