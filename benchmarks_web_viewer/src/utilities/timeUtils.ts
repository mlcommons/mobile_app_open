import { format } from "date-fns";

function formatDuration(seconds: number): string {
  let intSeconds = Math.ceil(seconds);
  let minutes = Math.floor(intSeconds / 60);
  intSeconds -= minutes * 60;
  const hours = Math.floor(minutes / 60);
  minutes -= hours * 60;

  const tokens: string[] = [];
  if (hours !== 0) {
    tokens.push(hours.toString().padStart(2, "0"));
  }
  tokens.push(minutes.toString().padStart(2, "0"));
  tokens.push(intSeconds.toString().padStart(2, "0"));

  return tokens.join(":");
}

function formatDateTime(value: Date): string {
  const dateFormat = "yyyy-MM-dd HH:mm:ss";
  return format(value, dateFormat);
}

export { formatDuration, formatDateTime };
