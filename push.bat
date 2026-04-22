@echo off
chcp 65001 > nul

echo Copyright Notice
echo © 2025 Yoyo-ace1110. All Rights Reserved.

:: 讀取使用者輸入作為提交訊息
set /p commit_msg="請輸入提交訊息 (Commit Message): "

echo.
echo ===================================================
echo 正在準備 Git 操作...
echo 提交訊息: "%commit_msg%"
echo ===================================================
echo.

:: 執行 git add .
echo [1/3] 執行 git add . (暫存所有變更)
git add .
if errorlevel 1 (
    echo.
    echo ❌ 錯誤: git add 失敗。請檢查專案狀態。
    goto :eof
)
echo.

:: 執行 git commit -m
echo [2/3] 執行 git commit
:: 檢查提交訊息是否為空
if "%commit_msg%"=="" (
    echo.
    echo ⚠️ 警告: 提交訊息為空，使用預設訊息 "Auto commit"
    set commit_msg=Auto commit
)

:: 執行提交
git commit -m "%commit_msg%"

:: Git commit 在沒有變更時會返回非零代碼，這不是真正的錯誤，需要特別處理
if errorlevel 1 (
    git status | findstr /i "nothing to commit"
    if not errorlevel 1 (
        echo.
        echo ✅ 成功: 沒有任何新的變更需要提交。跳過 Push。
        goto :eof
    ) else (
        echo.
        echo ❌ 錯誤: git commit 失敗。
        goto :eof
    )
)
echo.

:: 執行 git push
echo [3/3] 執行 git push (推送到遠端)
git push

if errorlevel 1 (
    echo.
    echo ❌ 推送失敗: 
    echo ---------------------------------------------------
    echo 請注意: 如果遠端有新變更，您需要先執行 git pull。
    echo ---------------------------------------------------
) else (
    echo.
    echo ===================================================
    echo 🎉 推送成功！
    echo ===================================================
)

pause
:eof
