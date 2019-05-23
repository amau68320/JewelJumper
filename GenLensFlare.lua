local colors = {
    { 170, 255,   0 }, -- Vert
    { 255, 203,   0 }, -- Orange
    { 255, 84 , 226 }, -- Violet
    { 84 , 152, 255 }, -- Bleu
}

local PROBA_MULTICOLORE = 0.05
local ALPHA_MIN = 30
local ALPHA_MAX = 75
local L_MIN = 0.5
local L_MAX = 4.0
local SZ_MIN = 40.0
local SZ_MAX = 250.0
local PROBA_TAILLE_ALEA = 0.25
local COUNT = 20

local function formatFloat(n)
    local n = math.floor(n * 10.0) / 10.0

    if n == math.floor(n) then
        return tostring(n) .. ".0f"
    else
        return tostring(n) .. "f"
    end
end

local function frandom(min, max)
    return math.random() * (max - min) + min
end

math.randomseed(os.time())

for i = 1, COUNT do
    local pos = math.random()
    local line = "{ " .. formatFloat(pos * (L_MAX - L_MIN) + L_MIN) .. ", "

    if math.random() < PROBA_TAILLE_ALEA then
        line = line .. formatFloat(frandom(SZ_MIN, SZ_MAX)) .. ", "
    else
        line = line .. formatFloat(pos * (SZ_MAX - SZ_MIN) + SZ_MIN) .. ", "
    end

    if math.random() < PROBA_MULTICOLORE then
        line = line .. "{ 255, 255, 255, " .. ALPHA_MAX .. " }, 2.0f },"
    else
        local color = colors[math.random(#colors)]
        local alpha = math.random(ALPHA_MIN, ALPHA_MAX)
        local kind  = math.random(0, 2)

        if kind == 2 then
            kind = 3
        end

        line = line .. "{ " .. color[1] .. ", " .. color[2] .. ", " .. color[3] .. ", " .. alpha .. " }, "
        line = line .. formatFloat(kind) .. " },"
    end

    print(line)
end
