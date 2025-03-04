import { defineStore } from 'pinia'

export const useRoomsStore = defineStore('rooms', {
    state: () => ({
        rooms: [],
        current_room: null
    }),
    actions: {
        setRooms(data) {
            this.rooms = data
        },
        clearRooms() {
            this.$reset()
            localStorage.removeItem('rooms')
        },
        addRoom(room) {
            this.rooms.push(room)
        },
        removeRoom(remove_room_id) {
            this.rooms = this.rooms.filter(room => room.room_id !== remove_room_id)
        },
        setCurrentRoom(room) {
            this.current_room = room
        },
        clearCurrentRoom() {
            this.current_room = null
        }
    },
    persist: true,
})
