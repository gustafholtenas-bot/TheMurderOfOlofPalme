#pragma once

#include "Modules/ModuleManager.h"

class FTMOPEngineModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
