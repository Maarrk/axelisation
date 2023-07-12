function has_duplicate_chars(str, ignore_case)
    if ignore_case then
        str = str:lower()
    end

    local seen = {}
    local duplicate_count = 0
    for i = 1, #str do
        local c = str:sub(i, i)

        if seen[c] then
            duplicate_count = duplicate_count + 1
        end
        seen[c] = true
    end
    return duplicate_count > 0, duplicate_count
end
