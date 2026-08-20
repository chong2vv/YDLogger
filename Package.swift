// swift-tools-version:5.9
import PackageDescription

let package = Package(
    name: "YDLogger",
    platforms: [.iOS(.v16)],
    products: [
        .library(name: "YDLog", targets: ["YDLog"]),
        .library(name: "YDLoggerUI", targets: ["YDLoggerUI"]),
    ],
    targets: [
        .target(
            name: "YDMmapCore",
            path: "Sources/YDMmapCore",
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("."),
            ]
        ),
        .target(
            name: "YDLog",
            dependencies: ["YDMmapCore"],
            path: "Sources/YDLog",
            exclude: ["YDMmapLogSwizzeling.m"],
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("."),
            ]
        ),
        .target(
            name: "YDLoggerUI",
            dependencies: ["YDLog"],
            path: "Sources/YDLoggerUI",
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("."),
            ]
        ),
    ],
    cxxLanguageStandard: .gnucxx17
)
