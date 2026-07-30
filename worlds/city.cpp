#include <cmath>
#include <string>

#include "engine/types.h"
#include "worlds/city.h"

namespace ASCIIpaper::Worlds {

    CityScene::CityScene(int width, int height, int carCount, int starCount, const std::string& weather, bool systemSync) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f), 
          m_carCount(carCount), m_starCount(starCount), m_weatherStr(weather),
          m_systemSync(systemSync),
          m_trainTimer(0.0f), m_trainX(-50.0f), m_trainActive(false),
          m_trainSpeed(45.0f), m_trainCars(3),
          m_rng(std::random_device{}()) {
        InitializeWorld();
    }

    void CityScene::InitializeWorld() {
        // Procedural Skyline
        std::uniform_int_distribution<int> widthDist(8, 16);
        std::uniform_int_distribution<int> heightDist(15, 35);
        std::uniform_int_distribution<int> colorDist(40, 80); 
        std::uniform_int_distribution<int> windowLitDist(0, 10); 
        
        int currentX = 2; 

        while (currentX < m_width - 5) {
            int bWidth = widthDist(m_rng);
            int bHeight = heightDist(m_rng);
            
            std::vector<bool> windows;
            for (int i = 0; i < bWidth * bHeight; ++i) {
                windows.push_back(windowLitDist(m_rng) == 0); 
            }

            m_buildings.push_back({
                currentX, bWidth, bHeight,
                static_cast<uint8_t>(colorDist(m_rng)), 
                static_cast<uint8_t>(colorDist(m_rng)), 
                static_cast<uint8_t>(colorDist(m_rng) + 20), 
                windows
            });

            currentX += (bWidth - 2); 
        }

        // Spawn Traffic
        std::uniform_real_distribution<float> speedDist(15.0f, 35.0f);
        std::uniform_int_distribution<int> carColorDist(150, 255);
        
        // Spawn cars evenly spaced to prevent immediate clumping
        float spacing = static_cast<float>(m_width) / m_carCount;

        for (int i = 0; i < m_carCount; ++i) {
            int lane = i % 4; // 0 to 3
            int yPos = (m_height - 16) + (lane * 2); // Lanes at -13, -11, -9, -7
            bool goesRight = (lane < 2); // Top two lanes go right, bottom two go left
            
            m_cars.push_back({
                (i * spacing), // Spread them out
                yPos, 
                speedDist(m_rng),
                goesRight ? Engine::Direction::Right : Engine::Direction::Left, 
                static_cast<uint8_t>(carColorDist(m_rng)),
                static_cast<uint8_t>(carColorDist(m_rng)),
                static_cast<uint8_t>(carColorDist(m_rng))
            });
        }

        // Spawn Stars
        std::uniform_int_distribution<int> starXDist(2, m_width - 3);
        std::uniform_int_distribution<int> starYDist(2, m_height - 25);
        std::uniform_real_distribution<float> timeDist(0.0f, 20.0f);
        std::uniform_real_distribution<float> thresholdDist(10.0f, 20.0f); // Twinkle every 10-20s

        for (int i = 0; i < m_starCount; ++i) {
            m_stars.push_back({
                starXDist(m_rng), starYDist(m_rng), 
                timeDist(m_rng), thresholdDist(m_rng), false
            });
        }

        std::uniform_real_distribution<float> ufoSpawnDist(60.0f, 120.0f); // 1 to 2 minutes
        m_ufo.state = UfoState::Waiting;
        m_ufo.timer = 0.0f;
        m_ufo.spawnThreshold = ufoSpawnDist(m_rng);

        // Weather system
        Engine::WeatherType wType = Engine::WeatherType::None;
        if (m_weatherStr == "rain") wType = Engine::WeatherType::Rain;
        else if (m_weatherStr == "snow") wType = Engine::WeatherType::Snow;
        else if (m_weatherStr == "storm") wType = Engine::WeatherType::Storm;
        
        m_weather.Initialize(wType, m_width, m_height);
    }

    void CityScene::Update(float deltaTime) {
        m_timeAccumulator += deltaTime;

        float cpuMultiplier = 1.0f;

        /*
         * Poll the system monitor if sync is enabled
         * We use CPU usage to dynamically scale car and train speed, and lightning frequency
         * We use RAM usage to increase building lights
         */
        if (m_systemSync) {
            m_sysMonitor.Update(deltaTime);
            float currentCpu = m_sysMonitor.GetCpuUsage(); // 0.0 to 100.0
            float currentRam = m_sysMonitor.GetRamUsage(); // 0.0 to 100.0

            // Base CPU Multiplier for cars (0% = 1x speed, 100% = 4x speed)
            cpuMultiplier = 1.0f + (currentCpu / 33.3f); 

            // Link RAM to Skyscraper Lights
            if (std::abs(currentRam - m_lastRamUsage) > 1.0f) {
                m_lastRamUsage = currentRam;
                int litChance = static_cast<int>(currentRam);

                for (auto& building : m_buildings) {
                    std::uniform_int_distribution<int> dist(0, 100);

                    for (size_t i = 0; i < building.windows.size(); ++i) {
                        building.windows[i] = (dist(m_rng) < litChance);
                    }
                }
            }

            // Link CPU to Storm Intensity
            // 0% CPU = Strikes every 10-20s. 100% CPU = Strikes every 1-3s!
            float minFreq = std::max(1.0f, 10.0f - (9.0f * (currentCpu / 100.0f)));
            float maxFreq = std::max(3.0f, 20.0f - (17.0f * (currentCpu / 100.0f)));
            m_weather.SetLightningFrequency(minFreq, maxFreq);
        }
        
        m_weather.Update(deltaTime);

        // Update Stars
        for (auto& star : m_stars) {
            star.timer += deltaTime;
            if (star.isTwinkling) {
                if (star.timer > 0.5f) { // Twinkle flare lasts 0.5 seconds
                    star.isTwinkling = false;
                    star.timer = 0.0f;
                }
            } else {
                if (star.timer > star.threshold) {
                    star.isTwinkling = true;
                    star.timer = 0.0f;
                }
            }
        }

        // Update Train
        if (!m_trainActive) {
            m_trainTimer += deltaTime;
            if (m_trainTimer >= 20.0f) { 
                m_trainActive = true;
                m_trainX = -60.0f; 
                m_trainTimer = 0.0f;

                // Link Train properties to hardware
                if (m_systemSync) {
                    // High RAM = Massive freight train (up to 9 cars)
                    m_trainCars = 3 + static_cast<int>(m_sysMonitor.GetRamUsage() / 15.0f); 
                    // High CPU = Bullet train
                    m_trainSpeed = 45.0f + m_sysMonitor.GetCpuUsage(); 
                } else {
                    m_trainCars = 3;
                    m_trainSpeed = 45.0f;
                }
            }
        } else {
            m_trainX += m_trainSpeed * deltaTime; 
            if (m_trainX > m_width + 20) m_trainActive = false; 
        }

        // Traffic
        for (auto& car : m_cars) {
            // Apply the CPU multiplier directly to the speed!
            car.x += car.speed * cpuMultiplier * static_cast<float>(car.direction) * deltaTime;
        }

        // Collision Check to prevent cars from passing through each other
        for (size_t i = 0; i < m_cars.size(); ++i) {
            for (size_t j = 0; j < m_cars.size(); ++j) {
                if (i == j) continue;
                
                // If they are in the same lane
                if (m_cars[i].y == m_cars[j].y && m_cars[i].direction == m_cars[j].direction) {
                    float dist = m_cars[j].x - m_cars[i].x;
                    
                    if (m_cars[i].direction == Engine::Direction::Right) {
                        // If Car J is directly in front of Car I
                        if (dist > 0.0f && dist < 6.0f) {
                            m_cars[i].x = m_cars[j].x - 6.0f; // Slow down
                            m_cars[i].speed = m_cars[j].speed; // Match speed of traffic
                        }
                    } else { // Left
                        if (dist < 0.0f && dist > -6.0f) {
                            m_cars[i].x = m_cars[j].x + 6.0f;
                            m_cars[i].speed = m_cars[j].speed;
                        }
                    }
                }
            }
        }

        // Screen Wrap
        for (auto& car : m_cars) {
            if (car.direction == Engine::Direction::Right && car.x > m_width + 4) {
                car.x = -4.0f;
                // Give it a new random speed when it wraps around so traffic changes
                std::uniform_real_distribution<float> speedDist(15.0f, 35.0f);
                car.speed = speedDist(m_rng); 
            } else if (car.direction == Engine::Direction::Left && car.x < -4) {
                car.x = static_cast<float>(m_width + 4);
                std::uniform_real_distribution<float> speedDist(15.0f, 35.0f);
                car.speed = speedDist(m_rng);
            }
        }

        // UFO State Machine
        if (m_ufo.state == UfoState::Waiting) {
            m_ufo.timer += deltaTime;
            if (m_ufo.timer >= m_ufo.spawnThreshold) {
                m_ufo.state = UfoState::Entering;
                m_ufo.x = -10.0f; // Start off-screen left
                m_ufo.y = 8.0f;   // High up in the sky
            }
        } else if (m_ufo.state == UfoState::Entering) {
            m_ufo.x += 35.0f * deltaTime; // Coast into the scene
            
            // Reached the center of the screen
            if (m_ufo.x >= m_width / 2.0f) { 
                m_ufo.state = UfoState::Hovering;
                m_ufo.timer = 0.0f; // Reuse timer to track hover duration
            }
        } else if (m_ufo.state == UfoState::Hovering) {
            m_ufo.timer += deltaTime;
            // Smoothly bob up and down while scanning
            m_ufo.y = 8.0f + std::sin(m_timeAccumulator * 2.5f) * 1.5f; 
            
            if (m_ufo.timer >= 12.0f) { // Hover and explore for 12 seconds
                m_ufo.state = UfoState::Leaving;
            }
        } else if (m_ufo.state == UfoState::Leaving) {
            // Fly away quickly
            m_ufo.x += 120.0f * deltaTime; 
            m_ufo.y -= 25.0f * deltaTime; 
            
            if (m_ufo.x > m_width + 10.0f) {
                m_ufo.state = UfoState::Waiting;
                m_ufo.timer = 0.0f;
                // Generate a new random wait time for the next visit
                std::uniform_real_distribution<float> ufoSpawnDist(60.0f, 120.0f);
                m_ufo.spawnThreshold = ufoSpawnDist(m_rng);
            }
        }
    }

    void CityScene::Draw(Engine::CharacterGrid& grid) {
        grid.Clear();

        // Stars
        for (const auto& star : m_stars) {
            if (star.isTwinkling) {
                grid.SetCell(star.x, star.y, '+', 255, 255, 255); // Bright twinkle
            } else {
                grid.SetCell(star.x, star.y, '.', 80, 80, 80); // Dim background star
            }
        }

        // Moon
        grid.SetCell(m_width - 25, 10, 'C', 255, 255, 200);

        // Buildings
        for (const auto& building : m_buildings) {
            int winIndex = 0;

            for (int y = 0; y < building.height; ++y) {
                int drawY = m_height - 20 - y;

                for (int x = 0; x < building.width; ++x) {
                    int drawX = building.x + x;

                    if (x % 2 == 1 && y % 2 == 1 && building.windows[winIndex]) {
                        grid.SetCell(drawX, drawY, '#', 255, 255, 100); 
                    } else {
                        grid.SetCell(drawX, drawY, '#', building.r, building.g, building.b); 
                    }
                    
                    winIndex++;
                }
            }
        }

        // Highway
        for (int x = 0; x < m_width; ++x) {
            if (x % 4 != 0) { 
                grid.SetCell(x, m_height - 17, '-', 100, 100, 100); // Top shoulder
                grid.SetCell(x, m_height - 15, '-', 80, 80, 80);    // Divider
                grid.SetCell(x, m_height - 13, '=', 200, 150, 50);  // Double Yellow
                grid.SetCell(x, m_height - 11, '-', 80, 80, 80);    // Divider
                grid.SetCell(x, m_height - 9, '-', 100, 100, 100);  // Bottom shoulder
            }
        }

        // Cars
        for (const auto& car : m_cars) {
            int ix = static_cast<int>(car.x);
            int iy = car.y;
            grid.SetCell(ix, iy, '[', car.r, car.g, car.b);
            grid.SetCell(ix + 1, iy, 'o', 180, 180, 180); 
            grid.SetCell(ix + 2, iy, '-', car.r, car.g, car.b);
            grid.SetCell(ix + 3, iy, 'o', 180, 180, 180); 
            grid.SetCell(ix + 4, iy, ']', car.r, car.g, car.b);
        }

        // Train tracks
        for (int x = 0; x < m_width; ++x) {
            grid.SetCell(x, m_height - 5, '_', 100, 70, 50); // Wooden ties
            if (x % 3 == 0) grid.SetCell(x, m_height - 5, '|', 150, 150, 150); // Steel rails
        }

        // Train
        if (m_trainActive) {
            int tx = static_cast<int>(m_trainX);
            int ty = m_height - 7;
            
            std::string trainArt = "";
            for (int i = 0; i < m_trainCars; ++i) trainArt += "[_]-"; // Cargo cars
            trainArt += "[_]>"; // Engine car
            
            for (size_t i = 0; i < trainArt.length(); ++i) {
                grid.SetCell(tx + static_cast<int>(i), ty, trainArt[i], 200, 200, 220); 
            }
        }

        // UFO
        if (m_ufo.state != UfoState::Waiting) {
            int ux = static_cast<int>(m_ufo.x);
            int uy = static_cast<int>(m_ufo.y);
            
            /* Draw a classic flying saucer: /(o)\ */ 
            grid.SetCell(ux, uy, '/', 100, 255, 100);
            grid.SetCell(ux + 1, uy, '(', 200, 200, 200);
            grid.SetCell(ux + 2, uy, 'o', 50, 255, 255); // Glowing cyan core
            grid.SetCell(ux + 3, uy, ')', 200, 200, 200);
            grid.SetCell(ux + 4, uy, '\\', 100, 255, 100);
            
            // Project a tiny scanner beam while hovering
            if (m_ufo.state == UfoState::Hovering) {
                grid.SetCell(ux + 2, uy + 1, 'v', 50, 255, 255);
            }
        }

        // Weather
        m_weather.Draw(grid, m_height - 20);

        // System monitor
        if (m_systemSync) {
            std::string hudStr = "SYSTEM SYNC | CPU: " + std::to_string(static_cast<int>(m_sysMonitor.GetCpuUsage())) + 
                                 "% | RAM: " + std::to_string(static_cast<int>(m_sysMonitor.GetRamUsage())) + "%";
            for (size_t i = 0; i < hudStr.length(); ++i) {
                grid.SetCell(2 + static_cast<int>(i), 1, hudStr[i], 0, 255, 255); // Glowing Cyan text in top left
            }
        }
    }

} // namespace ASCIIpaper::Worlds