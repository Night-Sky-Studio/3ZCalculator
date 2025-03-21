async function test_get_dmg() {
    const start = performance.now()

    const response = await fetch("http://localhost:5101/get_dmg", {
        method: "POST",
        body: JSON.stringify({
            aid: 1091,
            wid: 14109,
            discs: [
                {
                    id: 31000,
                    rarity: 4,
                    stats: [4, 3, 14, 13, 7],
                    levels: [15, 1, 0, 2, 2]
                },
                {
                    id: 32800,
                    rarity: 4,
                    stats: [8, 17, 7, 14, 13],
                    levels: [15, 2, 0, 2, 1]
                },
                {
                    id: 31000,
                    rarity: 4,
                    stats: [12, 13, 17, 14, 16],
                    levels: [15, 0, 2, 2, 0]
                },
                {
                    id: 32800,
                    rarity: 4,
                    stats: [14, 4, 3, 13, 16],
                    levels: [15, 1, 1, 3, 0]
                },
                {
                    id: 32800,
                    rarity: 4,
                    stats: [7, 3, 13, 14, 17],
                    levels: [15, 1, 2, 1, 0]
                },
                {
                    id: 32800,
                    rarity: 4,
                    stats: [7, 4, 14, 8, 13],
                    levels: [15, 1, 1, 2, 1]
                },
            ],
            rotation: 1
        })
    })

    const end = performance.now()

    console.log(end - start)
}

async function test_stop() {
    const response = await fetch("https://localhost:8080/stop", {
        method: "GET"
    })
}