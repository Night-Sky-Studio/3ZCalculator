const response = await fetch("http://localhost:8080/get_dmg", {
    method: "POST",
    body: JSON.stringify({
        aid: 1091,
        wid: 14109,
        discs: [
            {
                id: 31000,
                rarity: "S",
                stats: [4, 3, 14, 13, 7],
                levels: [15, 1, 0, 2, 2]
            },
            {
                id: 32800,
                rarity: "S",
                stats: [8, 17, 7, 14, 13],
                levels: [15, 2, 0, 2, 1]
            },
            {
                id: 31000,
                rarity: "S",
                stats: [12, 13, 17, 14, 16],
                levels: [15, 0, 2, 2, 0]
            },
            {
                id: 32800,
                rarity: "S",
                stats: [14, 4, 3, 13, 16],
                levels: [15, 1, 1, 3, 0]
            },
            {
                id: 32800,
                rarity: "S",
                stats: [7, 3, 13, 14, 17],
                levels: [15, 1, 2, 1, 0]
            },
            {
                id: 32800,
                rarity: "S",
                stats: [7, 4, 14, 8, 13],
                levels: [15, 1, 1, 2, 1]
            },
        ],
        rotation: 1091
    })
})

const json = await response.json()