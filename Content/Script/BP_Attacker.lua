--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

local BaseLookUpRate = 45.0

---@type BP_Attacker_C
local M = UnLua.Class()

function M:MoveForward(AxisValue)
	local Direction = self:GetActorForwardVector()
	self:AddMovementInput(Direction, AxisValue, false)
end

function M:MoveRight(AxisValue)
	local Direction = self:GetActorRightVector()
	self:AddMovementInput(Direction, AxisValue, false)
end

return M
