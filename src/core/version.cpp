#include <plugify/version.hpp>

using namespace plugify;

Version::Version(uint8_t major, uint8_t minor, uint8_t patch, uint8_t tweak) noexcept : _major{major}, _minor{minor}, _patch{patch}, _tweak{tweak} {
}

Version::Version(uint32_t version) noexcept {
	*this = version;
}

Version::operator uint32_t() const noexcept {
	return PLUGIFY_MAKE_VERSION(_major, _minor, _patch, _tweak);
}

auto Version::operator<=>(const Version& rhs) const noexcept {
	return PLUGIFY_MAKE_VERSION(_major, _minor, _patch, _tweak) <=> PLUGIFY_MAKE_VERSION(rhs._major, rhs._minor, rhs._patch, rhs._tweak);
}

Version& Version::operator=(uint32_t version) noexcept {
	_major = PLUGIFY_MAKE_VERSION_MAJOR(version);
	_minor = PLUGIFY_MAKE_VERSION_MINOR(version);
	_patch = PLUGIFY_MAKE_VERSION_PATCH(version);
	_tweak = PLUGIFY_MAKE_VERSION_TWEAK(version);
	return *this;
}

std::string Version::ToString() const {
	return std::format("{}.{}.{}.{}", _major, _minor, _patch, _tweak);
}
