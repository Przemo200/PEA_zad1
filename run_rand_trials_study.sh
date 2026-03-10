#!/usr/bin/env bash

set -euo pipefail

APP="./build/pea_zad1"
OUT_DIR="results"
OUT_CSV="${OUT_DIR}/rand_trials_study.csv"
TMP_CFG="${OUT_DIR}/tmp_rand_config.txt"

TRIALS=(100 500 1000 2000 5000 10000 20000 50000 500000)
REPEATS=5
BASE_SEED=12345

mkdir -p "${OUT_DIR}"

echo "set_type,instance_name,instance_file,note_trials,repeat_id,seed,best_cost,opt_cost,error_percent,time_ms" > "${OUT_CSV}"

run_list() {
    local list_file="$1"
    local set_type="$2"

    while IFS=';' read -r name instance_file opt_tour_file opt_cost; do
        # pomijaj komentarze i puste linie
        [[ -z "${name}" ]] && continue
        [[ "${name}" =~ ^# ]] && continue

        for trials in "${TRIALS[@]}"; do
            for rep in $(seq 1 "${REPEATS}"); do
                local seed=$((BASE_SEED + trials * 100 + rep))

                {
                    echo "mode=rand"
                    echo "instance_file=${instance_file}"
                    if [[ -n "${opt_tour_file:-}" ]]; then
                        echo "opt_tour_file=${opt_tour_file}"
                    elif [[ -n "${opt_cost:-}" ]]; then
                        echo "single_opt_cost=${opt_cost}"
                    fi
                    echo "rand_trials=${trials}"
                    echo "seed=${seed}"
                    echo "progress=false"
                    echo "show_matrix=false"
                } > "${TMP_CFG}"

                output="$("${APP}" "${TMP_CFG}")"

                best_cost="$(echo "${output}" | grep -m1 "Najlepszy koszt:" | sed 's/.*Najlepszy koszt: //')"
                opt_value="$(echo "${output}" | grep -m1 "Koszt optymalny / best known:" | sed 's/.*Koszt optymalny \/ best known: //' | sed 's/ .*//')"
                error_percent="$(echo "${output}" | grep -m1 "Blad wzgledny \[%\]:" | sed 's/.*Blad wzgledny \[%\]: //')"
                time_ms="$(echo "${output}" | grep -m1 "Czas \[ms\]:" | sed 's/.*Czas \[ms\]: //')"

                echo "${set_type},${name},${instance_file},${trials},${rep},${seed},${best_cost},${opt_value},${error_percent},${time_ms}" >> "${OUT_CSV}"

                echo "[OK] ${set_type} | ${name} | trials=${trials} | rep=${rep}"
            done
        done
    done < "${list_file}"
}

run_list "config/heuristics_tsp.txt" "TSP"
run_list "config/heuristics_atsp.txt" "ATSP"

rm -f "${TMP_CFG}"

echo
echo "Zakonczono. Wyniki zapisano do: ${OUT_CSV}"