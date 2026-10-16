/*
 * Canonical SimDeviceType / SimDevice ordering.
 *
 * Apple's simctl lists device types and devices in a fixed,
 * release-oriented order that is not derivable from any on-disk
 * metadata (verified by brute-forcing every key in the device type
 * Info.plist and profile.plist, and by searching the CoreSimulator
 * framework, its XPC service, and the simctl binaries). This table
 * records that observed order so the open-source simctl can
 * reproduce it. The order is grouped by product family (iPhone,
 * iPad, Apple TV, Apple Watch, Apple Vision, iPod) and within a
 * family follows Apple's newest-first release ordering.
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

static const char *const kSimDeviceTypeOrder[] = {
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17e",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-Air",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-16-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-16-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-16e",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-16",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-16-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-15-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-15-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-15",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-15-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-14-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-14-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-14",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-14-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-SE-3rd-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-13-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-13-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-13",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-13-mini",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-12-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-12-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-12",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-12-mini",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-SE--2nd-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-11-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-11-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-11",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-XR",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-XS",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-XS-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-X",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-8",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-8-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-7",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-7-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-SE",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-6s",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-6s-Plus",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-13-inch-M5-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-13-inch-M5-12GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-M5-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-M5-12GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-M4-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-M4-8GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-13-inch-M4-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-13-inch-M4-8GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-13-inch-M4",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-11-inch-M4",
	"com.apple.CoreSimulator.SimDeviceType.iPad-A16",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-13-inch-M3",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-11-inch-M3",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-11-inch-M2",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-13-inch-M2",
	"com.apple.CoreSimulator.SimDeviceType.iPad-mini-A17-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-4th-generation-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-4th-generation-8GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-12-9-inch-6th-generation-16GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-12-9-inch-6th-generation-8GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-10th-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-5th-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-mini-6th-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-3rd-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-12-9-inch-5th-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-9th-generation",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air--4th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad--8th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air--3rd-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-mini--5th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--11-inch---2nd-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--12-9-inch---4th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--11-inch-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--12-9-inch---3rd-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad--7th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--10-5-inch-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--12-9-inch---2nd-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad--6th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro--9-7-inch-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPad--5th-generation-",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-2",
	"com.apple.CoreSimulator.SimDeviceType.iPad-mini-4",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-3rd-generation-4K",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-3rd-generation-1080p",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-2nd-generation-4K",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-2nd-generation-1080p",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-4K",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-4K-1080p",
	"com.apple.CoreSimulator.SimDeviceType.Apple-TV-1080p",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-11-46mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-11-42mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-10-46mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-10-42mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-9-45mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-9-41mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Ultra-3-49mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Ultra-2-49mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-3-44mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-3-40mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-8-45mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-8-41mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Ultra-49mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-44mm-2nd-generation",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-40mm-2nd-generation",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-7-45mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-7-41mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-6-44mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-6-40mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-44mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-SE-40mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-5-44mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-5-40mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-4-44mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-4-40mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-3-42mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-3-38mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-2-42mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Watch-Series-2-38mm",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Vision-Pro-4K",
	"com.apple.CoreSimulator.SimDeviceType.Apple-Vision-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPod-touch--7th-generation-",
};

static const int kSimDeviceTypeOrderCount =
	(int)(sizeof kSimDeviceTypeOrder / sizeof kSimDeviceTypeOrder[0]);

/*
 * Observed default-device ordering.
 *
 * When listing devices, CoreSimulator groups them by runtime and then
 * orders them by device type. That ordering is close to, but not
 * identical to, the device type catalog order above (for example the
 * iPad mini is listed before the iPad Air). This table records the
 * device-type ordering observed in Apple's `simctl list devices`;
 * identifiers absent from it fall back to the catalog order above.
 */
static const char *const kSimDeviceDefaultOrder[] = {
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17-Pro-Max",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17e",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-Air",
	"com.apple.CoreSimulator.SimDeviceType.iPhone-17",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-13-inch-M5-12GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Pro-11-inch-M5-12GB",
	"com.apple.CoreSimulator.SimDeviceType.iPad-mini-A17-Pro",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-13-inch-M4",
	"com.apple.CoreSimulator.SimDeviceType.iPad-Air-11-inch-M4",
	"com.apple.CoreSimulator.SimDeviceType.iPad-A16",
};

static const int kSimDeviceDefaultOrderCount =
	(int)(sizeof kSimDeviceDefaultOrder /
	sizeof kSimDeviceDefaultOrder[0]);
