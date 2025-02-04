#include "halley/resources/resource.h"
#include "halley/support/exception.h"

using namespace Halley;

Resource::~Resource() = default;

void Resource::setMeta(Metadata m)
{
	// Only allow setting meta once to avoid a race condition
	// For example, resource_collection.cpp would try to set a meta on a Texture while texture.cpp is trying to load it and referencing its data
	if (!metaSet) {
		meta = std::move(m);
		metaSet = true;
	}
}

void Resource::setAssetId(String id)
{
	assetId = std::move(id);
}

void Resource::onLoaded(Resources& resources)
{
}

void Resource::increaseAssetVersion()
{
	++assetVersion;
}

void Resource::reloadResource(Resource&& resource)
{
	++assetVersion;
	resource.assetVersion = assetVersion;
	reload(std::move(resource));
}

ResourceMemoryUsage Resource::getMemoryUsage() const
{
	return ResourceMemoryUsage{};
}

void Resource::increaseAge(float time)
{
	age += time;
}

void Resource::resetAge()
{
	age = 0;
}

float Resource::getAge() const
{
	return age;
}

void Resource::reload(Resource&& resource)
{
}

ResourceObserver::ResourceObserver()
{
}

ResourceObserver::ResourceObserver(const Resource& res)
{
	startObserving(res);
}

ResourceObserver::~ResourceObserver()
{
	stopObserving();
}

void ResourceObserver::startObserving(const Resource& r)
{
	res = &r;
	assetVersion = res->getAssetVersion();
}

void ResourceObserver::stopObserving()
{
	res = nullptr;
	assetVersion = 0;
}

bool ResourceObserver::needsUpdate() const
{
	return res && res->getAssetVersion() != assetVersion;
}

void ResourceObserver::update()
{
	assetVersion = res->getAssetVersion();
}

const Resource* ResourceObserver::getResourceBeingObserved() const
{
	return res;
}

AsyncResource::AsyncResource() 
	: failed(false)
	, loading(false)
{}

AsyncResource::~AsyncResource()
{
	waitForLoad(true);
}

AsyncResource::AsyncResource(const AsyncResource& other)
{
	other.waitForLoad(true);
	*this = other;
}

AsyncResource::AsyncResource(AsyncResource&& other) noexcept
{
	other.waitForLoad(true);
	*this = std::move(other);
}

AsyncResource& AsyncResource::operator=(const AsyncResource& other)
{
	other.waitForLoad(true);
	failed.store(other.failed);
	Resource::operator=(static_cast<const Resource&>(other));
	return *this;
}

AsyncResource& AsyncResource::operator=(AsyncResource&& other) noexcept
{
	other.waitForLoad(true);
	failed.store(other.failed);
	Resource::operator=(static_cast<Resource&&>(other));
	return *this;
}

void AsyncResource::startLoading()
{
	loading = true;
	failed = false;
}

void AsyncResource::doneLoading()
{
	if (loading) {
		Vector<Promise<void>> promises;
		{
			std::unique_lock<std::mutex> lock(loadMutex);
			loading = false;
			promises = std::move(pendingPromises);
		}
		loadWait.notify_all();
		for (auto& p: promises) {
			p.set();
		}
	}
}

void AsyncResource::loadingFailed()
{
	failed = true;
	doneLoading();
}

void AsyncResource::waitForLoad(bool acceptFailed) const
{
	if (loading) {
		std::unique_lock<std::mutex> lock(loadMutex);
		while (loading) {
			loadWait.wait(lock);
		}
	}
	if (failed && !acceptFailed) {
		throw Exception("Resource failed to load.", HalleyExceptions::Resources);
	}
}

Future<void> AsyncResource::onLoad() const
{
	std::unique_lock<std::mutex> lock(loadMutex);
	if (loading) {
		pendingPromises.push_back({});
		return pendingPromises.back().getFuture();
	} else {
		return Future<void>::makeImmediate({});
	}
}

bool AsyncResource::isLoaded() const
{
	return !loading;
}

bool AsyncResource::hasSucceeded() const
{
	return !failed;
}

bool AsyncResource::hasFailed() const
{
	return failed;
}
