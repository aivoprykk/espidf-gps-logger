import axios from "axios";

export default axios.create({
  baseURL: "http://esp-logger.local/api/v1",
  headers: {
    "Content-type": "application/json"
  },
  withCredentials: false
});
