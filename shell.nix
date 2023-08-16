{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = with pkgs; [
        # clang_10
        python39
        python27
        valgrind
        # lld
        # libcxx


        llvmPackages_10.clangUseLLVM
        llvmPackages_10.llvm
        llvmPackages_10.openmp
        llvmPackages_10.lld
        llvmPackages_10.bintools
        llvmPackages_10.libcxx
        llvmPackages_10.libcxxabi
        llvmPackages_10.compiler-rt
        llvmPackages_10.libunwind


        # Just random utilities
        which
        git
        lldb_10
        hyperfine
        kcachegrind
        graphviz
        rr
        linuxPackages_latest.perf
    ];

    permittedInsecurePackages = [
        "python-2.7.18.6"
    ];

    # Since we're playing god, let's disable all extra c wrapper stuff
    hardeningDisable = [ "all" ];
}
