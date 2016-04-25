# Otto

`otto` is an early build tool from iMatix Corporation, written in
Perl v4.  It is implemented using a state machine, for which the
implementation (in `otto.d`) is built using `libero` to process the
state machin source (in `otto.l`); `otto.fmt` was written by hand
to support the various `otto` actions on a given platform.

When run, `otto` generates native build scripts for the platforms that
it supports:

*   UNIX

*   MSDOS

*   DEC VMS

*   OS/2

which can then be run stand alone, providing the `c` script wrapper
for the platform is present in the `PATH`.

Brief documentation is available in [otto.man](otto.man); apart from
this the only documentation is the source code, and `otto` examples
for various early iMatix projects (see <https://github.com/imatix-legacy>).

`otto.zip` contains the original release artefact (the only release
every made stand alone); the other files were unpacked from `otto.zip`
for ease of reference.

Released under the GNU GPL v2, a copy of which can be found in 
[copying](copying).

NOTE: more modern iMatix projects use other, more modern, build processes.
