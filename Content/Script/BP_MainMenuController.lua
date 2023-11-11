--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type BP_MainMenuController_C
local M = UnLua.Class()

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

--创建开始UI
function M:ReceiveBeginPlay()
    local Widget = UE.UWidgetBlueprintLibrary.Create(self, UE.UClass.Load("/Game/Blueprint/UI/UI_Main.UI_Main_C"))
    Widget:AddToViewport()
    self.UI_Main = Widget
    self.bShowMouseCursor = true
    -- self.Overridden.ReceiveBeginPlay(self)
end

-- function M:ReceiveEndPlay()
-- end

-- function M:ReceiveTick(DeltaSeconds)
-- end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

return M
