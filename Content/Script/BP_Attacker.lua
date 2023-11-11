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

--输入绑定
function M:MoveForward(AxisValue)
	self:InputMoveForward(AxisValue)
end

function M:MoveRight(AxisValue)
	self:InputMoveRight(AxisValue)
end

function M:Turn(AxisValue)
	self:InputTurn(AxisValue)
end

function M:LookUp(AxisValue)
	self:InputLookUp(AxisValue)
end

function M:Jump_Pressed()
	self:JumpAction()
end

function M:Jump_Released()
	self:StopJumpAction()
end

function M:LowSpeedWalk_Pressed()
	self:LowSpeedWalkAction()
end

function M:LowSpeedWalk_Released()
	self:NormalSpeedWalkAction()
end

function M:Fire_Pressed()
	self:InputFirePressed()
end

function M:Fire_Released()
	self:InputFireReleased()
end

function M:Reload_Pressed()
	self:InputReload()
end

function M:Aiming_Pressed()
	self:InputAimingPressed()
end

function M:Aiming_Released()
	self:InputAimingReleased()
end

return M
