group "default" {
  targets = ["android"]
}

target "android" {
  context    = "flutter/android/docker"
  dockerfile = "Dockerfile"
  // platforms = ["linux/amd64"]  // optionally set platforms here
}
