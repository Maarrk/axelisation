const std = @import("std");
const raylib_build = @import("raylib/src/build.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "axelisation",
        .root_source_file = .{ .path = "src/main.c" },
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath("raylib/src");
    b.installArtifact(exe);

    const raylib = raylib_build.addRaylib(b, target, optimize);
    exe.linkLibrary(raylib);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
