EPWA = 'https://awiacja.imgw.pl/metar00.php?airport=EPWA'
EPMO = 'https://awiacja.imgw.pl/metar00.php?airport=EPMO'
EPKK = 'https://awiacja.imgw.pl/metar00.php?airport=EPKK'

airport_url = {
    -- EPWA = 'https://awiacja.imgw.pl/metar00.php?airport=EPWA',
    -- EPMO = 'https://awiacja.imgw.pl/metar00.php?airport=EPMO',
    -- EPKK = 'https://awiacja.imgw.pl/metar00.php?airport=EPKK',
}

function airport_to_url(name)
    return 'https://awiacja.imgw.pl/metar00.php?airport=' .. name
end

-- Actually the coolest is generating the table using function
airports = { 'EPWA', 'EPMO', 'EPKK' }
for _, airport in ipairs(airports) do
    airport_url[airport] = airport_to_url(airport)
end
