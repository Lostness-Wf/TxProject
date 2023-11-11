--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type UI_Main_C
local M = UnLua.Class()

--Space to begin 动画
function M:Construct()
    self:PlayAnimation(self.Blink, 0, 0)
end

return M
