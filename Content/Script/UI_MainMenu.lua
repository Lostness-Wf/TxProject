--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type UI_MainMenu_C
local M = UnLua.Class()

--绑定按键委托
function M:Construct()
    self.QuitGameButton.OnClicked:Add(self, self.OnClicked_ExitButton)
    self.CreateGameButton.OnClicked:Add(self, self.OnClicked_CreateGameButton)
    self.FindgameButton.OnClicked:Add(self, self.OnClicked_FindgameButton)
end

function M:OnClicked_ExitButton()
    local PlayerController = self:GetOwningPlayer()
    UE.UKismetSystemLibrary.QuitGame(self, PlayerController, 0, false)
end

function M:OnClicked_CreateGameButton()
    -- UE.UGameplayStatics.OpenLevel(self, "TestMap2")
end

function M:OnClicked_FindgameButton()
    UE.UKismetSystemLibrary.ExecuteConsoleCommand(self, "open 43.138.16.144:7777")
end

--解绑按键委托
function M:Destruct()
    self.QuitGameButton.OnClicked:Remove(self, self.OnClicked_ExitButton)
    self.CreateGameButton.OnClicked:Remove(self, self.OnClicked_CreateGameButton)
    self.FindgameButton.OnClicked:Remove(self, self.OnClicked_FindgameButton)
end

return M
