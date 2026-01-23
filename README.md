# ticketverify-engine

> **ticketverify-engine** est un moteur natif **C++20**, exposé via un **CLI contractuel**, chargé de transformer du **texte OCR brut** en **données structurées métier**.
>
> Il est conçu comme une **black box déterministe**, découplée du backend, versionnable et industrialisable.

---

## 🎯 Objectif

Fournir un moteur :

* indépendant du backend
* isolé techniquement
* testable
* versionnable
* remplaçable

Le backend **ne contient aucune logique de parsing métier**.
Il orchestre, sécurise et mappe le résultat.

---

## 🧠 Philosophie

Le moteur est pensé comme un composant système :

* **stateless**
* **déterministe**
* **sans dépendance réseau**
* **sans état persistant**
* **sans logique applicative**

Il transforme une entrée brute → sortie structurée.

---

## 🔌 Contrat CLI

### Entrée

```txt
stdin (UTF-8)
```

Texte OCR brut (iOS Vision / ML Kit / autres moteurs).

---

### Sortie

```json
{
  "status": "ok|partial|reject",
  "confidence": 0.98,
  "total": { "value": 12.34, "currency": "EUR" },
  "merchant": "CARREFOUR CITY",
  "warnings": []
}
```

---

### Règles strictes

* `stdout` → JSON uniquement
* `stderr` → logs uniquement
* UTF-8 strict
* exit codes cohérents
* aucun log sur stdout
* aucun état interne

---

## ⚙️ Responsabilités

### Parsing

* normalisation OCR
* nettoyage du bruit
* segmentation logique

### Extraction

* merchant (headers multi-lignes)
* total TTC (formats réels)
* devise

### Détection de signaux

* mots-clés carte
* TVA
* SIRET

---

## 🧪 Qualité

* unit tests
* fixtures OCR réelles
* tests d’intégration CLI
* robustesse UTF-8
* gestion erreurs

---

## 🔁 Intégration backend

```
Spring Boot
  ↓
ProcessBuilder
  ↓
stdin UTF-8
  ↓
ticketverify-engine
  ↓
stdout JSON
  ↓
mapping domaine
```

---

## 🧱 Build

```bash
cmake -S . -B build
cmake --build build
```

---

## 📦 Packaging

Le binaire est conçu pour :

* packaging indépendant
* versioning
* signature
* checksum
* déploiement contrôlé

---

## 🎯 Positionnement

Ce moteur n’est pas un outil utilitaire.

C’est un **composant système** autonome, qui permet :

* évolution indépendante
* remplacement sans impact backend
* isolation des risques
* spécialisation métier

---
